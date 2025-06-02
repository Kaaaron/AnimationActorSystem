// Copyright 2025 Aaron Kemner, All Rights reserved.


#include "AnimNotifyState_SpawnActorBase.h"

#include "AnimationActorSubsystem.h"
#include "AnimationActorSystem.h"
#include "Animation/MirrorDataTable.h"
#include "Engine/AssetManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StreamableManager.h"

void UAnimNotifyState_SpawnActorBase::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                  float TotalDuration,
                                                  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	const TSoftClassPtr<UObject> SpawnableClass = GetSpawnableClassToLoad();

	if (!SpawnableClass)
	{
		UE_LOG(LogAnimActorSys, Error, TEXT("Actor Class is invalid for %s"), *StaticPartialAnimActorGuid.ToString());
		return;
	}

	UWorld* World = MeshComp->GetWorld();
	if (!ensureMsgf(World, TEXT("Triggered NotifyBegin without World. How did you do this?")))
	{
		return;
	}

	const FGuid SpawnGuid = ConstructDeterministicGuidFromComponent(MeshComp);
#if WITH_EDITORONLY_DATA
	EditorCachedNotifyData.CachedDeterministicGuid = SpawnGuid;
	EditorCachedNotifyData.MeshComp = MeshComp;
	EditorCachedNotifyData.Animation = Animation;
	EditorCachedNotifyData.TotalDuration = TotalDuration;
	EditorCachedNotifyData.EventReference = EventReference;
#endif
	auto ClassLoaded = [this,
		SpawnableClass,
		NotifyAttachTransform = AttachTransform,
		SpawnGuid,
		MeshComp,
		Animation,
		TotalDuration,
		EventReference]
		{
			if (UAnimationActorSubsystem* SubSys = UAnimationActorSubsystem::GetFromMesh(MeshComp))
			{
				AActor* SpawnedActor = SubSys->SpawnAnimActor(SpawnableClass.Get(),
					NotifyAttachTransform,
					SpawnGuid);
				if(SpawnedActor) // May be nullptr, for example if the world is tearing down
				{
					PostSpawnActor(SpawnedActor, SubSys, MeshComp, Animation, TotalDuration, EventReference);
				}
			}
		};

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	switch (GetLoadingBehaviour())
	{
		case EAnimActorClassLoadingBehaviour::BeginPlay_Async:
		case EAnimActorClassLoadingBehaviour::FirstTimeRequested_Async:
			if (SpawnableClass.IsValid())
			{
				ClassLoaded();
			}
			else
			{
				StreamableManager.RequestAsyncLoad(SpawnableClass.ToSoftObjectPath(),
					FStreamableDelegate::CreateWeakLambda(MeshComp, ClassLoaded));
			}
			break;
		default:
			StreamableManager.RequestSyncLoad(SpawnableClass.ToSoftObjectPath());
			ClassLoaded();
	}
}

void UAnimNotifyState_SpawnActorBase::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UAnimationActorSubsystem* SubSys = UAnimationActorSubsystem::GetFromMesh(MeshComp))
	{
		SubSys->DestroyAnimActor(ConstructDeterministicGuidFromComponent(MeshComp));
	}
}

#if WITH_EDITOR
void UAnimNotifyState_SpawnActorBase::OnAnimNotifyCreatedInEditor(FAnimNotifyEvent& ContainingAnimNotifyEvent)
{
	Super::OnAnimNotifyCreatedInEditor(ContainingAnimNotifyEvent);

	StaticPartialAnimActorGuid = FGuid::NewGuid();
}

void UAnimNotifyState_SpawnActorBase::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const UWorld* World = EditorCachedNotifyData.MeshComp ? EditorCachedNotifyData.MeshComp->GetWorld() : nullptr;
	if(!World)
	{
		return;
	}
	
	if (UAnimationActorSubsystem* SubSys = World->GetSubsystem<UAnimationActorSubsystem>())
	{
		if(EditorCachedNotifyData.CachedDeterministicGuid.IsValid())
		{
			SubSys->DestroyAnimActor(EditorCachedNotifyData.CachedDeterministicGuid);
			if(AActor* SpawnedActor = SubSys->SpawnAnimActor(
					GetSpawnableClassToLoad().LoadSynchronous(),
					AttachTransform,
					EditorCachedNotifyData.CachedDeterministicGuid))
			{
				PostSpawnActor(SpawnedActor,
				   SubSys,
				   EditorCachedNotifyData.MeshComp,
				   EditorCachedNotifyData.Animation,
				   EditorCachedNotifyData.TotalDuration,
				   EditorCachedNotifyData.EventReference);
			}
		}
	}
}
#endif

void UAnimNotifyState_SpawnActorBase::PostSpawnActor(AActor* SpawnedActor, UAnimationActorSubsystem* Subsystem,
                                                     USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                     float TotalDuration,
                                                     const FAnimNotifyEventReference& EventReference)
{
	if(!SpawnedActor || !SpawnedActor->GetRootComponent())
	{
		return;
	}
	SpawnedActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
	
	// Only KeepRelative makes sense here. With AttachTransform being Identity this would be SnapToTarget,
	// and KeepWorld is mostly meaningless here.
	const FAttachmentTransformRules Rule = FAttachmentTransformRules(EAttachmentRule::KeepRelative,
	                                                                 bWeldSimulatedBodies);
	FName MirroredBone = NAME_None;
	if(const UMirrorDataTable* MDT = EventReference.GetMirrorDataTable())
	{
		MirroredBone = MDT->GetSettingsMirrorName(AttachBone);
	}
	FName BoneToUse = MirroredBone == NAME_None ? AttachBone : MirroredBone;

	SpawnedActor->AttachToComponent(MeshComp, Rule, BoneToUse);
}

FString UAnimNotifyState_SpawnActorBase::BuildNotifyNameFromObject(UObject* Object) const
{
	static FString NoneString = FString(TEXT("None"));
	if (AttachBone != NAME_None)
	{
		return FString::Printf(TEXT("Spawn %s on %s"), Object ? *Object->GetName() : *NoneString,
		                       *AttachBone.ToString());
	}
	return FString::Printf(TEXT("Spawn %s"), Object ? *Object->GetName() : *NoneString);
}

FGuid UAnimNotifyState_SpawnActorBase::ConstructDeterministicGuidFromComponent(USkeletalMeshComponent* InComponent) const
{
	// Ideally, I'd just get the ActorGuid, but sadly that one is editor-only, so no use for my purposes...
	const FGuid DynamicPartialAnimActorGuid = FGuid::NewDeterministicGuid(InComponent->GetPathName());
	return FGuid::Combine(StaticPartialAnimActorGuid, DynamicPartialAnimActorGuid);
}
