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
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AnimActor")
	TObjectPtr<USkeletalMesh> MeshToSpawn = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AnimActor")
	bool bOverrideCollisionProfile = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="OverrideCollisionProfile", EditConditionHides), Category="AnimActor")
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
