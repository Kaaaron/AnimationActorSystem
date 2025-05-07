// Copyright 2025 Aaron Kemner, All Rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "AnimNotifyState_SpawnActorBase.h"
#include "AnimationActorSystemSettings.h"
#include "AnimNotifyState_SpawnActorOfClass.generated.h"

/**
 * Spawn an actor of a given class on NotifyBegin and destroy it when the notify ends.
 */
UCLASS(DisplayName="Timed Spawn Actor of Class")
class ANIMATIONACTORSYSTEM_API UAnimNotifyState_SpawnActorOfClass : public UAnimNotifyState_SpawnActorBase
{
	GENERATED_BODY()

public:
	UAnimNotifyState_SpawnActorOfClass()
	{
#if WITH_EDITORONLY_DATA
		NotifyColor = FColor::Magenta;
#endif
	}
		
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="AnimActor")
	TSoftClassPtr<AActor> ClassToSpawn = nullptr;

	virtual TSoftClassPtr<UObject> GetSpawnableClassToLoad() override
		{ return ClassToSpawn; };

	virtual EAnimActorClassLoadingBehaviour GetLoadingBehaviour() override
		{ return UAnimationActorSystemSettings::Get()->ActorClassLoadingBehaviour; };

#if WITH_EDITOR
	virtual FString GetNotifyName_Implementation() const override;
#endif
};
