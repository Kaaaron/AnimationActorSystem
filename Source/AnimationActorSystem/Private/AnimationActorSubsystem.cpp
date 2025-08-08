// Copyright 2025 Aaron Kemner, All Rights reserved.


#include "AnimationActorSubsystem.h"

#include "AnimationActorSystem.h"
#include "AnimationActorSystemSettings.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"

FName UAnimationActorSubsystem::SpawnedAnimActorTag = FName(TEXT("AnimActor"));

AActor* UAnimationActorSubsystem::SpawnAnimActor(const TSubclassOf<AActor>& Class, const FTransform& Transform,
                                                 const FGuid Guid)
{	
	if (!Class)
	{
		return nullptr;
	}

	ReferencedAnimActorClasses.AddUnique(Class);

	UWorld* World = GetWorld();
	checkf(World, TEXT("WorldSubsystems should not be able to exist without world."));

	if(World->bIsTearingDown)
	{
		return nullptr;
	}
	
	if(AnimActorSys::FActorCounter* FoundCounter = SpawnedActors.Find(Guid))
	{
		if(AActor* Actor = FoundCounter->Increment())
		{
			return Actor;
		}
	}
	
	FActorSpawnParameters Params = FActorSpawnParameters();
	Params.ObjectFlags = Params.ObjectFlags & RF_Transient;
	AActor* SpawnedActor = World->SpawnActor(Class, &Transform, Params);
	SpawnedActors.Emplace(Guid, AnimActorSys::FActorCounter(SpawnedActor)).Increment();
	SpawnedActor->Tags.AddUnique(SpawnedAnimActorTag);
	return SpawnedActor;
	
}

AActor* UAnimationActorSubsystem::GetAnimActorByGuid(const FGuid& GuidToLookFor) const
{
	if (const AnimActorSys::FActorCounter* Counter = SpawnedActors.Find(GuidToLookFor))
	{
		return Counter->GetActor();
	}
	return nullptr;
}

void UAnimationActorSubsystem::DestroyAnimActor(const FGuid Guid)
{
	if (AnimActorSys::FActorCounter* ActorCounter = SpawnedActors.Find(Guid))
	{		
		AActor* Actor = ActorCounter->RemoveSingle();
		
		if(!bool(*ActorCounter))
		{
			if(IsValid(Actor)) // Check bc maybe this actor has been destroyed already from an outside system.
			{
				Actor->Destroy();
			}
			
			SpawnedActors.Remove(Guid);
		}
		else
		{
			UE_LOG(LogAnimActorSys, Display, TEXT("Requested destruction of AnimActor for Guid %s, but but it is still active %d times"), *Guid.ToString(), ActorCounter->GetCount())
		}
	}
	else
	{
		UE_LOG(LogAnimActorSys, Warning, TEXT("Requested destruction of AnimActor for Guid %s, but this Guid has no Counter."), *Guid.ToString())
	}
}

void UAnimationActorSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	const UAnimationActorSystemSettings* Settings = UAnimationActorSystemSettings::Get();	
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	
	// Load SkeletalMeshActor Class if applicable
	if (Settings->SkeletalMeshActorLoadingBehaviour == EAnimActorClassLoadingBehaviour::BeginPlay_Async)
	{
		StreamableManager.RequestAsyncLoad(Settings->SkeletalMeshActorClass.ToSoftObjectPath(),
		                                   FStreamableDelegate::CreateWeakLambda(this, [this]
		                                   {
			                                   ReferencedAnimActorClasses.AddUnique(
				                                   UAnimationActorSystemSettings::Get()->SkeletalMeshActorClass.Get());
		                                   }));
	}
	else if (Settings->SkeletalMeshActorLoadingBehaviour == EAnimActorClassLoadingBehaviour::BeginPlay_Blocking)
	{
		ReferencedAnimActorClasses.AddUnique(Settings->SkeletalMeshActorClass.LoadSynchronous());
	}

	// Load StaticMeshActor Class  if applicable
	if (Settings->StaticMeshActorLoadingBehaviour == EAnimActorClassLoadingBehaviour::BeginPlay_Async)
	{
		StreamableManager.RequestAsyncLoad(Settings->StaticMeshActorClass.ToSoftObjectPath(),
		                                   FStreamableDelegate::CreateWeakLambda(this, [this]
		                                   {
			                                   ReferencedAnimActorClasses.AddUnique(
			                                   	UAnimationActorSystemSettings::Get()->StaticMeshActorClass.Get());
		                                   }));
	}
	else if (Settings->StaticMeshActorLoadingBehaviour == EAnimActorClassLoadingBehaviour::BeginPlay_Blocking)
	{
		ReferencedAnimActorClasses.AddUnique(Settings->StaticMeshActorClass.LoadSynchronous());
	}
}

UAnimationActorSubsystem* UAnimationActorSubsystem::Get(const UObject* WorldContext)
{
	if(!WorldContext)
	{
		UE_LOG(LogAnimActorSys, Error, TEXT("Tried to get AnimationActorSubsystem from invalid WorldContext Object."))
		return nullptr;
	}
	
	UWorld* World = WorldContext->GetWorld();
	if (!ensureMsgf(World, TEXT("Please ensure to only query the AnimationActorSubsystem only for supported world types.")))
	{
		return nullptr;
	}

	return World->GetSubsystem<UAnimationActorSubsystem>();
}

bool UAnimationActorSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	// This Subsystem is generally pretty lightweight, and it doesn't tick, so I'd rather have it be active, in case
	// a notify needs it, rather than not.
	// If not desired, the Notify should not fire instead of this not supporting a given world type.
	return !(WorldType == EWorldType::Type::None
		|| WorldType == EWorldType::Type::Editor);
}
