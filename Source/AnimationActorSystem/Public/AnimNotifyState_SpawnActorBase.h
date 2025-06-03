// Copyright 2025 Aaron Kemner, All Rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "AnimationActorSystem.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimationActorTypes.h"
#include "Animation/MirrorDataTable.h"
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
	UAnimNotifyState_SpawnActorBase()
	{
#if WITH_EDITORONLY_DATA
		bShouldFireInEditor = true;
#endif
	}
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
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
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

private:
#if WITH_EDITORONLY_DATA
	/** Cached Data for use in the Animation Editor to be able to react to property changes
	 * when the Notify is already in progress. */
	struct FCachedNotifyData
	{
		FCachedNotifyData() = delete;
		FCachedNotifyData(USkeletalMeshComponent* SourceMeshComp,
			UAnimSequenceBase* SourceAnimation,
			const float SourceTotalDuration,
			const FAnimNotifyEventReference& SourceEventReference)
			:MeshComp(SourceMeshComp),
			Animation(SourceAnimation),
			TotalDuration(SourceTotalDuration),
			WeakEventReference(SourceEventReference)
		{}
		
		TWeakObjectPtr<USkeletalMeshComponent> MeshComp = nullptr;
		TWeakObjectPtr<UAnimSequenceBase> Animation = nullptr;
		float TotalDuration = 0.f;

		/** Partial Data from FAnimNotifyEventReference but with TObjectPtr being switched to TWeakObjectPtr */
		struct FWeakAnimNotifyEventReference
		{
			explicit FWeakAnimNotifyEventReference(const FAnimNotifyEventReference& SourceEventReference)
				:MirrorTable(SourceEventReference.GetMirrorDataTable()),
				NotifySource(SourceEventReference.GetSourceObject()),
				CurrentAnimTime(SourceEventReference.GetCurrentAnimationTime()),
				bActiveContext(SourceEventReference.IsActiveContext())
			{}

			/** Converts to a reconstructed version of the original EventReference used to construct this one,
			 * excluding Contextual data like what FAnimNotifyEventReference gets from the TickRecord. */
			FAnimNotifyEventReference ToEventReference() const
			{
				return FAnimNotifyEventReference(nullptr, NotifySource.Get(), MirrorTable.Get());
			}
			
			/** If set, the Notify has been mirrored. */
			TWeakObjectPtr<const UMirrorDataTable> MirrorTable = nullptr; 

			/** The source object of this Notify (e.g. AnimSequence) */
			TWeakObjectPtr<const UObject> NotifySource = nullptr;

			/** The recorded time from the tick record that this notify event was fired at */
			float CurrentAnimTime = 0.0f;

			/** Whether the context this notify was fired from is active or not (active == not blending out). */
			bool bActiveContext = false;
		} WeakEventReference;
	};
	TMap<FGuid, FCachedNotifyData> EditorCachedNotifyData;
#endif
};
