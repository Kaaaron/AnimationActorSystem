// Copyright 2025 Aaron Kemner, All Rights reserved.


#include "AnimNotifyState_SpawnActorBase.h"

#include "AnimationActorSubsystem.h"
#include "AnimationActorSystem.h"
#include "Animation/AnimNotifyLibrary.h"
#include "Animation/MirrorDataTable.h"
#include "Animation/AnimSequenceBase.h"
#include "Engine/AssetManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StreamableManager.h"

void UAnimNotifyState_SpawnActorBase::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                  float TotalDuration,
                                                  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (!(MeshComp && Animation))
	{
		return;
	}
	
	const TSoftClassPtr<AActor> SpawnableClass = GetSpawnableClassToLoad();

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
	AActor* Owner = MeshComp->GetOwner();
	if (Owner && Owner->ActorHasTag(UAnimationActorSubsystem::SpawnedAnimActorTag))
	{
		return;
	}

	const FGuid SpawnGuid = ConstructDeterministicGuidFromComponent(MeshComp);
	
#pragma region EditorOnlyPreview
#if WITH_EDITOR
	if (UAnimationActorSubsystem* SubSys = UAnimationActorSubsystem::Get(MeshComp))
	{
		const FAnimNotifyEvent* Notify = EventReference.GetNotify();
		const UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
		const bool bIsEditorWorld = SubSys->GetWorld()->WorldType == EWorldType::Type::Editor
					|| SubSys->GetWorld()->WorldType == EWorldType::Type::EditorPreview;
		if (bIsEditorWorld && Notify && AnimInst && AnimInst->GetActiveMontageInstance())
		{
			const float ActiveMontagePosition = AnimInst->GetActiveMontageInstance()->GetPosition();
			const bool bPlayheadIsWithinNotifyWindow = ActiveMontagePosition <= Notify->GetEndTriggerTime() &&
					ActiveMontagePosition >= Notify->GetTriggerTime();
			if (bPlayheadIsWithinNotifyWindow
				&& SubSys->GetAnimActorByGuid(SpawnGuid))
			{
				/** For some reason Unreal Handles NotifyStates differently when scrubbing through an AnimSequence vs an AnimMontage.
				 * For an AnimSequence, the NotifyState starts when entering the NotifyWindow, and ends when exiting the window, ticking inbetween.
				 * For AnimMontages though, Unreal will repeatedly fire a Start=>Tick=>End Pattern during scrubbing, and not do anything
				 * while within the NotifyWindow if not actively scrubbing.
				 * So instead of spawning a new AnimActor here in the Montage Case, we simply return if we already spawned one.
				 * This should hack around the described issue and may even save performance in-editor when trying to spawn particularly heavy
				 * meshes/actors from these states.
				 */
				return;
			}
			if (!bPlayheadIsWithinNotifyWindow)
			{
				return;
			}
		}
	}
#endif
#pragma endregion
	
	TWeakObjectPtr<USkeletalMeshComponent> WeakMeshComp(MeshComp);
	TWeakObjectPtr<UAnimSequenceBase> WeakAnimation(Animation);
	AnimActorSys::FWeakAnimNotifyEventReference WeakEventRef(EventReference);
#if WITH_EDITORONLY_DATA
	FCachedNotifyData CachedData = EditorCachedNotifyData.Emplace(SpawnGuid,
		{MeshComp, Animation, TotalDuration, EventReference});	
#endif

	auto ClassLoaded = [this,
		SpawnableClass,
		NotifyAttachTransform = AttachTransform,
		SpawnGuid,
		WeakMeshComp,
		WeakAnimation,
		TotalDuration,
		WeakEventRef]
		{
			USkeletalMeshComponent* MeshComp_Local = WeakMeshComp.Get();
			UAnimSequenceBase* Animation_Local = WeakAnimation.Get();
			UAnimationActorSubsystem* SubSys_Local = UAnimationActorSubsystem::Get(MeshComp_Local);
			if (!SpawnableClass || !MeshComp_Local || !Animation_Local || !SubSys_Local)
			{
				UE_LOG(LogAnimActorSys, Error, TEXT("Failed to spawn AnimActor (%s)."), SpawnableClass ? *SpawnableClass->GetName() : TEXT("InvalidClass"));
				return;
			}
		
			AActor* SpawnedActor = SubSys_Local->SpawnAnimActor(SpawnableClass.Get(),
																NotifyAttachTransform,
																SpawnGuid);
			if(SpawnedActor /**May still be nullptr, for example if the world is tearing down*/)
			{
				PostSpawnActor(SpawnedActor, SubSys_Local, MeshComp_Local, Animation_Local, TotalDuration, WeakEventRef.ToEventReference());
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

	const FGuid DeterministicGuid = ConstructDeterministicGuidFromComponent(MeshComp);
	if (UAnimationActorSubsystem* SubSys = UAnimationActorSubsystem::Get(MeshComp))
	{	
#pragma region EditorOnlyPreview
#if WITH_EDITOR
		const FAnimNotifyEvent* Notify = EventReference.GetNotify();
		const UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
		const bool bIsEditorWorld = SubSys->GetWorld()->WorldType == EWorldType::Type::Editor
					|| SubSys->GetWorld()->WorldType == EWorldType::Type::EditorPreview;
		if (bIsEditorWorld && Notify && AnimInst && AnimInst->GetActiveMontageInstance())
		{
			const float ActiveMontagePosition = AnimInst->GetActiveMontageInstance()->GetPosition();
			const bool bPlayheadIsWithinNotifyWindow = ActiveMontagePosition <= Notify->GetEndTriggerTime() &&
					ActiveMontagePosition >= Notify->GetTriggerTime();
			if (bPlayheadIsWithinNotifyWindow && SubSys->GetAnimActorByGuid(DeterministicGuid))
			{
				/** Info on what this does is above in NotifyBegin() in the EditorOnlyPreview region */
				return;
			}
		}
#endif
#pragma endregion
	
		SubSys->DestroyAnimActor(DeterministicGuid);
#if WITH_EDITORONLY_DATA
		EditorCachedNotifyData.Remove(DeterministicGuid);
#endif
	}
}

#if WITH_EDITOR
void UAnimNotifyState_SpawnActorBase::OnAnimNotifyCreatedInEditor(FAnimNotifyEvent& ContainingAnimNotifyEvent)
{
	Super::OnAnimNotifyCreatedInEditor(ContainingAnimNotifyEvent);

	StaticPartialAnimActorGuid = FGuid::NewGuid();
}

void UAnimNotifyState_SpawnActorBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const TMap<FGuid, FCachedNotifyData> EditorCachedNotifyData_Copy = EditorCachedNotifyData; // Copied to avoid modification during iteration
	for(const auto& [CachedGuid, CachedNotifyData] : EditorCachedNotifyData_Copy)
	{
		if (UAnimationActorSubsystem* SubSys = UAnimationActorSubsystem::Get(CachedNotifyData.MeshComp.Get()))
		{
			SubSys->DestroyAnimActor(CachedGuid);
			if(AActor* SpawnedActor = SubSys->SpawnAnimActor(
					GetSpawnableClassToLoad().LoadSynchronous(),
					AttachTransform,
					CachedGuid))
			{
				PostSpawnActor(SpawnedActor,
				   SubSys,
				   CachedNotifyData.MeshComp.Get(),
				   CachedNotifyData.Animation.Get(),
				  CachedNotifyData.TotalDuration,
				  CachedNotifyData.WeakEventReference.ToEventReference());
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
