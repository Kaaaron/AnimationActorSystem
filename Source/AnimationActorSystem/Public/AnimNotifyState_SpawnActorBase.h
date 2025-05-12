// Copyright 2025 Aaron Kemner, All Rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "AnimationActorSystem.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimationActorTypes.h"
#include "AnimNotifyState_SpawnActorBase.generated.h"


class UAnimationActorSubsystem;
class USkeletalMeshComponent;


/**
 * NotifyState baseclass to handle spawning and destroying actors from the AnimationActorSystem.
 * The general flow is like this:
 * => NotifyBegin()
 * => GetSpawnableClassToLoad()
 * => Load actor class
 * => Let UAnimationActorSubsystem spawn the actor
 * => PostSpawnActor()
 */
UCLASS(Abstract)
class ANIMATIONACTORSYSTEM_API UAnimNotifyState_SpawnActorBase : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	/** The bone or socket the spawned actor should be attached to */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, DisplayName="Attach to", meta = (AnimNotifyBoneName = "true"), Category="AnimActor")
	FName AttachBone = NAME_None;
	
	/** Whether to weld simulated bodies (see FAttachmentTransformRules::bWeldSimulatedBodies) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AnimActor")
	bool bWeldSimulatedBodies = false;
	
	/** Transform to apply relative to AttachBone, if specified */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AnimActor")
	FTransform AttachTransform = FTransform::Identity;
	
	virtual TSoftClassPtr<UObject> GetSpawnableClassToLoad() { return nullptr; };

	virtual EAnimActorClassLoadingBehaviour GetLoadingBehaviour()
		{ return EAnimActorClassLoadingBehaviour::FirstTimeRequested_Blocking; }

	/** Executed after the Actor is spawned and registered with the subsystem.
	 * Baseclass version already handles attachment, so don't forget the super:: call or do it yourself. */
	virtual void PostSpawnActor(AActor* SpawnedActor, UAnimationActorSubsystem* Subsystem,
	                            USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	                            const FAnimNotifyEventReference& EventReference);
	
	FString BuildNotifyNameFromObject(UObject* Object) const;

	/** Get the Guid the actor spawned by this notify will be identified by. */
	UFUNCTION(BlueprintPure, Category="AnimActor")
	FGuid GetAnimActorGuid() const
		{ return StaticPartialAnimActorGuid; }

#pragma region UAnimNotifyState Interface
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
							 const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
						   const FAnimNotifyEventReference& EventReference) override;
#if WITH_EDITOR
	virtual void OnAnimNotifyCreatedInEditor(FAnimNotifyEvent& ContainingAnimNotifyEvent) override;
#endif
#pragma endregion

#pragma region UObject Interface
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override
	{
		StaticPartialAnimActorGuid = FGuid::NewGuid();
		Super::PostDuplicate(DuplicateMode);
	};
#pragma endregion

protected:
	/** Guid the spawned actor gets associated with. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, AdvancedDisplay, meta=(DisplayPriority=100), Category="AnimActor")
	FGuid StaticPartialAnimActorGuid = FGuid();

	/** Constructs a deterministic FGuid from the StaticPartialAnimActorGuid and the object path of the InComponent.
	 * Using this instead of just the StaticPartialAnimActorGuid automatically differentiates between
	 * this notify being fired from the same animation but on different actors/components.
	 */
	FGuid ConstructDeterministicGuidFromComponent(USkeletalMeshComponent* InComponent) const;
};
