// Copyright 2025 Aaron Kemner, All Rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "AnimationActorSystemSettings.h"
#include "AnimNotifyState_SpawnActorBase.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/CollisionProfile.h"
#include "AnimNotifyState_SpawnStaticMesh.generated.h"

/**
 * Spawn a StaticMesh on NotifyBegin and destroy it when the notify ends.
 */
UCLASS(DisplayName="Timed Spawn StaticMesh")
class ANIMATIONACTORSYSTEM_API UAnimNotifyState_SpawnStaticMesh : public UAnimNotifyState_SpawnActorBase
{
	GENERATED_BODY()

public:
	UAnimNotifyState_SpawnStaticMesh()
	{
#if WITH_EDITORONLY_DATA
		NotifyColor = FColor(33,244,250);
#endif
	}

	/** The static mesh to spawn for the duration of this notify */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AnimActor")
	TObjectPtr<UStaticMesh> MeshToSpawn = nullptr;

	/** Whether to override the static mesh's collision profile */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AnimActor")
	bool bOverrideCollisionProfile = false;

	/** Override for the static mesh's collision profile */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition="bOverrideCollisionProfile"), Category="AnimActor")
	FCollisionProfileName CollisionProfileOverride = FCollisionProfileName();

#pragma region UAnimNotifyState_SpawnActorBase Interface
	virtual TSoftClassPtr<AActor> GetSpawnableClassToLoad() override
		{ return AStaticMeshActor::StaticClass();	};
	
	virtual EAnimActorClassLoadingBehaviour GetLoadingBehaviour() override
		{ return UAnimationActorSystemSettings::Get()->StaticMeshActorLoadingBehaviour; };

	virtual void PostSpawnActor(AActor* SpawnedActor, UAnimationActorSubsystem* Subsystem,
	                            USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	                            const FAnimNotifyEventReference& EventReference) override;
#pragma endregion UAnimNotifyState_SpawnActorBase Interface

#pragma region UAnimNotifyState Interface
	virtual FString GetNotifyName_Implementation() const override;
#pragma endregion
};
