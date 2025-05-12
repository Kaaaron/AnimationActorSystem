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

	/** The animation that should play on the notifies skeletal mesh */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AnimActor")
	TObjectPtr<UAnimSequenceBase> AnimationToPlay = nullptr;

	/** Whether to specify a loop behaviour for the animation.
	 * If false, will use the loop setting from the animation.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(InlineEditConditionToggle), Category="AnimActor")
	bool bOverrideLoopBehaviour = false;

	/** The override value for whether to loop the animation. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="bOverrideLoopBehaviour"), Category="AnimActor")
	bool bLoopAnimation = false;

	/** Whether to override the skeletal mesh's collision profile */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AnimActor")
	bool bOverrideCollisionProfile = false;

	/** Override for the skeletal mesh's collision profile */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="bOverrideCollisionProfile"), Category="AnimActor")
	FCollisionProfileName CollisionProfileOverride = FCollisionProfileName();

#pragma region UAnimNotifyState_SpawnActorBase Interface
	virtual TSoftClassPtr<UObject> GetSpawnableClassToLoad() override
		{ return ASkeletalMeshActor::StaticClass(); };
	
	virtual EAnimActorClassLoadingBehaviour GetLoadingBehaviour() override
		{ return UAnimationActorSystemSettings::Get()->SkeletalMeshActorLoadingBehaviour; };

	virtual void PostSpawnActor(AActor* SpawnedActor, UAnimationActorSubsystem* Subsystem,
	                            USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	                            const FAnimNotifyEventReference& EventReference) override;
#pragma endregion

#pragma region UAnimNotifyState Interface
	virtual FString GetNotifyName_Implementation() const override;
#pragma endregion
};
