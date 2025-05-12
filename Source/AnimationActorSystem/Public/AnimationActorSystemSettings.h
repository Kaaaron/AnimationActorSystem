// Copyright 2025 Aaron Kemner, All Rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "AnimationActorTypes.h"
#include "Engine/DeveloperSettings.h"
#include "Animation/SkeletalMeshActor.h"
#include "Engine/StaticMeshActor.h"
#include "AnimationActorSystemSettings.generated.h"

UCLASS(Config=Game, DefaultConfig)
class ANIMATIONACTORSYSTEM_API UAnimationActorSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:	
	
#pragma region Actor by Class
	/** How to load actors referenced by AnimNotifyState_SpawnActorOfClass.
	 * Is only allowed to be "FirstTimeRequested_[...]" since we don't know what classes to load on BeginPlay*/
	UPROPERTY(Config, BlueprintReadWrite, EditAnywhere, Category="Actor by Class")
	EAnimActorClassLoadingBehaviour ActorClassLoadingBehaviour =
			EAnimActorClassLoadingBehaviour::FirstTimeRequested_Async;
#pragma endregion

#pragma region Skeletal Meshes
	/** The SkeletalMeshActor class to spawn by UAnimNotifyState_SpawnSkeletalMeshActor. */
	UPROPERTY(Config, BlueprintReadWrite, EditAnywhere, Category="Skeletal Mesh")
	TSoftClassPtr<ASkeletalMeshActor> SkeletalMeshActorClass = ASkeletalMeshActor::StaticClass();

	/** How to load actors referenced by UAnimNotifyState_SpawnSkeletalMeshActor */
	UPROPERTY(Config, BlueprintReadWrite, EditAnywhere, Category="Skeletal Mesh")
	EAnimActorClassLoadingBehaviour SkeletalMeshActorLoadingBehaviour =
			EAnimActorClassLoadingBehaviour::FirstTimeRequested_Async;

	/** Can SkeletalMeshActors spawned via UAnimNotifyState_SpawnSkeletalMeshActor ever affect Navigation? */
	UPROPERTY(Config, BlueprintReadWrite, EditAnywhere, Category="Skeletal Mesh")
	bool bSkeletalCanAffectNavigation = true;
#pragma endregion

#pragma region Static Meshes
	/** The StaticMeshActor class to spawn by UAnimNotifyState_SpawnStaticMeshActor. */
	UPROPERTY(Config, BlueprintReadWrite, EditAnywhere, Category="Static Mesh")
	TSoftClassPtr<AStaticMeshActor> StaticMeshActorClass = AStaticMeshActor::StaticClass();

	/** How to load actors referenced by UAnimNotifyState_SpawnStaticMeshActor */
	UPROPERTY(Config, BlueprintReadWrite, EditAnywhere, Category="Static Mesh")
	EAnimActorClassLoadingBehaviour StaticMeshActorLoadingBehaviour =
			EAnimActorClassLoadingBehaviour::FirstTimeRequested_Async;

	/** Can StaticMeshActors spawned via UAnimNotifyState_SpawnStaticMeshActor ever affect Navigation? */
	UPROPERTY(Config, BlueprintReadWrite, EditAnywhere, Category="Static Mesh")
	bool bStaticCanAffectNavigation = true;
#pragma endregion

	static const UAnimationActorSystemSettings* Get()
		{ return GetDefault<UAnimationActorSystemSettings>(); };
	
#if WITH_EDITOR	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
#pragma region Settings Editor Display
	virtual FText GetSectionText() const override
	{
		return FText::FromString("Animation Actor System");
	};

	virtual FText GetSectionDescription() const override
	{
		return FText::FromString("Description?");
	};
#endif

	virtual FName GetSectionName() const override
	{
		return FName(TEXT("Animation Actor System"));
	}

	virtual FName GetCategoryName() const override
	{
		return FName(TEXT("Plugins"));
	}

	virtual FName GetContainerName() const override
	{
		return FName(TEXT("Project"));
	}
#pragma endregion
};
