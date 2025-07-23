// Copyright 2025 Aaron Kemner, All Rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "AnimNotifyState_SpawnActorBase.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/SkeletalMeshActor.h"
#include "AnimationActorSystemSettings.h"
#include "Engine/CollisionProfile.h"
#include "AnimNotifyState_SpawnSkeletalMesh.generated.h"

class UAnimSequenceBase;

/**
 * Spawn a SkeletalMesh on NotifyBegin and destroy it when the notify ends.
 */
UCLASS(DisplayName="Timed Spawn SkeletalMesh")
class ANIMATIONACTORSYSTEM_API UAnimNotifyState_SpawnSkeletalMesh : public UAnimNotifyState_SpawnActorBase
{
	GENERATED_BODY()

public:
	UAnimNotifyState_SpawnSkeletalMesh()
	{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(230,150,200);
#endif
	}

	/** The skeletal mesh to spawn for the duration of this notify */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AnimActor")
	TObjectPtr<USkeletalMesh> MeshToSpawn = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AnimActor")
	EAnimActorAnimationMode AnimationMode = EAnimActorAnimationMode::AnimSequence;

#pragma region EAnimActorAnimationMode::AnimSequence 
	/** The animation that should play on the notifies spawned skeletal mesh */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AnimActor", meta=(EditConditionHides,
		EditCondition="AnimationMode == EAnimActorAnimationMode::AnimSequence"))
	TObjectPtr<UAnimSequenceBase> AnimationToPlay = nullptr;

	/** Whether to specify a loop behaviour for the animation.
	 * If false, will use the loop setting from the animation.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditConditionHides,
		EditCondition="AnimationMode == EAnimActorAnimationMode::AnimSequence"), Category="AnimActor")
	bool bOverrideLoopBehaviour = false;

	/** The override value for whether to loop the animation. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditConditionHides,
		EditCondition="bOverrideLoopBehaviour && AnimationMode == EAnimActorAnimationMode::AnimSequence"), Category="AnimActor")
	bool bLoopAnimation = false;
#pragma endregion

#pragma region EAnimActorAnimationMode::AnimBlueprint
	/** The animation that should play on the notifies spawned skeletal mesh */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AnimActor", meta=(EditConditionHides,
		EditCondition="AnimationMode == EAnimActorAnimationMode::AnimBlueprint"))
	TSubclassOf<UAnimInstance> AnimationBlueprint = nullptr;
#pragma endregion

	/** Whether to override the skeletal mesh's collision profile */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AnimActor")
	bool bOverrideCollisionProfile = false;

	/** Override for the skeletal mesh's collision profile */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="bOverrideCollisionProfile", EditConditionHides), Category="AnimActor")
	FCollisionProfileName CollisionProfileOverride = FCollisionProfileName();

#pragma region UAnimNotifyState_SpawnActorBase Interface
	virtual TSoftClassPtr<AActor> GetSpawnableClassToLoad() override
		{ return UAnimationActorSystemSettings::Get()->SkeletalMeshActorClass; };
	
	virtual EAnimActorClassLoadingBehaviour GetLoadingBehaviour() override
		{ return UAnimationActorSystemSettings::Get()->SkeletalMeshActorLoadingBehaviour; };

	virtual void PostSpawnActor(AActor* SpawnedActor, UAnimationActorSubsystem* Subsystem,
	                            USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	                            const FAnimNotifyEventReference& EventReference) override;
#pragma endregion

#pragma region UAnimNotifyState Interface
	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
#pragma endregion
};
