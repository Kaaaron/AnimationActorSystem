// Copyright 2025 Aaron Kemner, All Rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "AnimationActorTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "AnimationActorSubsystem.generated.h"

class UWorld;

/**
 * Subsystem to manage spawning, tracking, and destroying AnimActors.
 */
UCLASS()
class ANIMATIONACTORSYSTEM_API UAnimationActorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	static UAnimationActorSubsystem* Get(const UObject* WorldContext);

	/** A tag put on all spawned AnimActors to be able to identify them. */
	static FName SpawnedAnimActorTag;
	
	AActor* SpawnAnimActor(const TSubclassOf<AActor>& Class, const FTransform& Transform, const FGuid Guid);
	[[nodiscard]] AActor* GetAnimActorByGuid(const FGuid& GuidToLookFor) const;
	void DestroyAnimActor(const FGuid Guid);

#pragma region UWorldSubsystem Interface
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
#pragma endregion
	
private:
	/** Spawned actors mapped as the GUID this system receives from the Notify to a counter of actor pointers. */
	TMap<FGuid, AnimActorSys::FActorCounter> SpawnedActors;

	/** List of referenced classes to hold onto, to prevent them from being GC'd */
	UPROPERTY(Transient)
	TArray<TSubclassOf<AActor>> ReferencedAnimActorClasses;
};
