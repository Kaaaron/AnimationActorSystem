// Copyright 2025 Aaron Kemner, All Rights reserved.


#include "AnimNotifyState_SpawnSkeletalMesh.h"

#include "AnimationActorSubsystem.h"
#include "AnimationActorSystem.h"
#include "Animation/AnimNotifyLibrary.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimSingleNodeInstance.h"

void UAnimNotifyState_SpawnSkeletalMesh::PostSpawnActor(AActor* SpawnedActor, UAnimationActorSubsystem* Subsystem,
                                                        USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                                        const FAnimNotifyEventReference& EventReference)
{
	Super::PostSpawnActor(SpawnedActor, Subsystem, MeshComp, Animation, TotalDuration, EventReference);

	const ASkeletalMeshActor* SKMA = CastChecked<ASkeletalMeshActor>(SpawnedActor);
	USkeletalMeshComponent* Comp = SKMA->GetSkeletalMeshComponent();
	check(Comp)
	Comp->SetSkeletalMesh(MeshToSpawn);
	
	switch (AnimationMode)
	{
	case EAnimActorAnimationMode::AnimSequence:
		{
			if(AnimationToPlay)
			{
				Comp->PlayAnimation(AnimationToPlay, bOverrideLoopBehaviour ? bLoopAnimation : AnimationToPlay->bLoop);
				Comp->SetPlayRate(0);
				Comp->InitAnim(false);
			}
			break;
		}
	case EAnimActorAnimationMode::PoseLeader:
		{
			Comp->SetLeaderPoseComponent(MeshComp);
			break;
		}
	case EAnimActorAnimationMode::AnimBlueprint:
		{
			Comp->SetAnimInstanceClass(AnimationBlueprint);
		}
	}
	
	if(bOverrideCollisionProfile)
	{
		Comp->SetCollisionProfileName(CollisionProfileOverride.Name, true);
	}
	
	Comp->SetCanEverAffectNavigation(UAnimationActorSystemSettings::Get()->bSkeletalCanAffectNavigation);
}

FString UAnimNotifyState_SpawnSkeletalMesh::GetNotifyName_Implementation() const
{
	return BuildNotifyNameFromObject(MeshToSpawn);
}

void UAnimNotifyState_SpawnSkeletalMesh::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	if (!MeshComp || !Animation)
	{
		return;
	}
	switch (AnimationMode)
	{
	case EAnimActorAnimationMode::AnimSequence:
		{
			if (const UAnimationActorSubsystem* Subsystem = UAnimationActorSubsystem::Get(MeshComp))
			{
				ASkeletalMeshActor* AnimActor = Cast<ASkeletalMeshActor>(Subsystem->GetAnimActorByGuid(ConstructDeterministicGuidFromComponent(MeshComp)));
				if (!AnimActor
					|| !AnimActor->GetSkeletalMeshComponent()
					|| !AnimActor->GetSkeletalMeshComponent()->GetSingleNodeInstance())
				{
					return;
				}
#if WITH_EDITOR
				// Handle case of being a preview for an AnimMontage
				const FAnimNotifyEvent* NotifyEvent = EventReference.GetNotify();
				const UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
				if (NotifyEvent && AnimInstance && AnimInstance->GetActiveMontageInstance())
				{
					const FAnimMontageInstance* ActiveMontage = AnimInstance->GetActiveMontageInstance();
					const float ActiveMontagePosition = ActiveMontage->GetPosition();
					const float NotifyTriggerTime = NotifyEvent->GetTriggerTime();
					const bool bPlayheadIsWithinNotifyWindow = ActiveMontagePosition <= NotifyEvent->GetEndTriggerTime() &&
							ActiveMontagePosition >= NotifyTriggerTime;
					if (bPlayheadIsWithinNotifyWindow)
					{
						AnimActor->GetSkeletalMeshComponent()->GetSingleNodeInstance()->SetPosition(FMath::Max(0.f, ActiveMontagePosition-NotifyTriggerTime));
						return;
					}
				}
#endif

				// Keep playing animation in sync with the animation that spawned the actor.
				// This is to prevent situations where the spawning actor has a separate time dilation set from the world
				// from de-syncing the animation of this actor.
				const float ElapsedTime = UAnimNotifyLibrary::GetCurrentAnimationNotifyStateTime(EventReference);
				AnimActor->GetSkeletalMeshComponent()->GetSingleNodeInstance()->SetPosition(ElapsedTime);
			}
			return;
		}
	default:
		{
			return;
		}
	}
}
