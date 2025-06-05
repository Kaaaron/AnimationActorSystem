// Copyright 2025 Aaron Kemner, All Rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimNotifyQueue.h"
#include "Animation/MirrorDataTable.h"

#include "AnimationActorTypes.generated.h"

UENUM(BlueprintType)
enum class EAnimActorClassLoadingBehaviour: uint8
{
	BeginPlay_Blocking			UMETA(DisplayName="BeginPlay (Blocking)"),
	BeginPlay_Async				UMETA(DisplayName="BeginPlay (Asynchronous)"),
	FirstTimeRequested_Blocking	UMETA(DisplayName="First Time Requested (Blocking)"),
	FirstTimeRequested_Async	UMETA(DisplayName="First Time Requested (Asynchronous)")
};

namespace AnimActorSys
{
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
			return Data.Get();		
		}

		AActor* RemoveSingle()
		{		
			Counter = FMath::Max(Counter-1, 0);
			AActor* RV = Data.Get();
			if(!Counter)
			{
				Data = nullptr;
			}
			return RV;
		}

		int GetCount() const
			 { return Counter; }
		
		explicit operator bool() const
			{ return Counter > 0;}
		
	private:
		TWeakObjectPtr<AActor> Data = nullptr;
		
		int Counter = 0;
	};
}
