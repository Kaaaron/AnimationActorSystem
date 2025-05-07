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
USTRUCT()
struct FActorCounter
{
	GENERATED_BODY()
public:
	FActorCounter() =  default;
	FActorCounter(AActor* InNewData)
	{
		Data = InNewData;
		Counter = 0;
	}
	
	void Add (AActor* InNewData)
	{
		check(InNewData);
		if(!Data)
		{
			Data = InNewData;
			return;
		}
		
		check(Data);
		if(!ensureAlwaysMsgf(Data == InNewData,
			TEXT("ActorCounter for %s getting new data for %s. Reassigning. May loose data."), *Data->GetName(), *InNewData->GetName()))
		{
			Data = InNewData;
			Counter = 0;
			return;
		}
		
		Counter++;
		
	}

	AActor* RemoveSingle()
	{		
		Counter = FMath::Max(Counter-1, 0);
		AActor* RV = Data;
		if(!Counter)
		{
			Data = nullptr;
		}
		return RV;
	}

	operator bool() const
		{ return bool(Counter);}
	
private:
	UPROPERTY(Transient)
	TObjectPtr<AActor> Data = nullptr;
	
	int Counter = 0;
};
