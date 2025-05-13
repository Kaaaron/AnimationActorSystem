// Copyright 2025 Aaron Kemner, All Rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AnimationActorTypes.generated.h"

UENUM(BlueprintType)
enum class EAnimActorClassLoadingBehaviour: uint8
{
	BeginPlay_Blocking			UMETA(DisplayName="BeginPlay (Blocking)"),
	BeginPlay_Async				UMETA(DisplayName="BeginPlay (Asynchronous)"),
	FirstTimeRequested_Blocking	UMETA(DisplayName="First Time Requested (Blocking)"),
	FirstTimeRequested_Async	UMETA(DisplayName="First Time Requested (Asynchronous)")
};

/**
 * Holds a pointer to an actor and a counter how often it has been added.
 */
struct FActorCounter
{
	FActorCounter() = delete;
	explicit  FActorCounter(AActor* InNewData)
	{
		Data = InNewData;
		Counter = 0;
	}
	
	AActor* Increment ()
	{
		Counter++;
		if(!Data.IsValid())
		{
			return nullptr;
		}
		return Data.Get(false);		
	}

	AActor* RemoveSingle()
	{		
		Counter = FMath::Max(Counter-1, 0);
		AActor* RV = nullptr;
		if(Data.IsValid())
		{
			RV = Data.Get();
		}
		if(!Counter)
		{
			Data = nullptr;
		}
		return RV;
	}

	operator bool() const
		{ return bool(Counter);}
	
private:
	TWeakObjectPtr<AActor> Data = nullptr;
	
	int Counter = 0;
};
