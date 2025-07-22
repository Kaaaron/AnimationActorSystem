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
#if WITH_EDITOR
				if (Subsystem->GetWorld()->WorldType == EWorldType::Type::Editor
					|| Subsystem->GetWorld()->WorldType == EWorldType::Type::EditorPreview)
				{
					// PlayRate in an Animation Editor is handled via the EditorOnly tick implementation of this NotifyState
					Comp->SetPlayRate(0);
				}
#endif
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

#if WITH_EDITOR
void UAnimNotifyState_SpawnSkeletalMesh::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if (!(MeshComp&&Animation))
	{
		return;
	}
	switch (AnimationMode)
	{
	case EAnimActorAnimationMode::AnimSequence:
		{
			if (const UAnimationActorSubsystem* Subsystem = UAnimationActorSubsystem::Get(MeshComp))
			{
				if (!(Subsystem->GetWorld()->WorldType == EWorldType::Type::Editor
					|| Subsystem->GetWorld()->WorldType == EWorldType::Type::EditorPreview))
				{
					return;
				}
				ASkeletalMeshActor* AnimActor = Cast<ASkeletalMeshActor>(Subsystem->GetAnimActorByGuid(ConstructDeterministicGuidFromComponent(MeshComp)));
				if (!AnimActor
					|| !AnimActor->GetSkeletalMeshComponent()
					|| !AnimActor->GetSkeletalMeshComponent()->GetSingleNodeInstance())
				{
					return;
				}

				// Handle case of being a preview for an AnimMontage
				const FAnimNotifyEvent* Notify = EventReference.GetNotify();
				const UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
				if (Notify && AnimInst && AnimInst->GetActiveMontageInstance())
				{
					const FAnimMontageInstance* ActiveMontage = AnimInst->GetActiveMontageInstance();
					const float ActiveMontagePosition = ActiveMontage->GetPosition();
					const float NotifyTriggerTime = Notify->GetTriggerTime();
					const bool bPlayheadIsWithinNotifyWindow = ActiveMontagePosition <= Notify->GetEndTriggerTime() &&
							ActiveMontagePosition >= NotifyTriggerTime;
					if (bPlayheadIsWithinNotifyWindow)
					{
						AnimActor->GetSkeletalMeshComponent()->GetSingleNodeInstance()->SetPosition(FMath::Max(0.f, ActiveMontagePosition-NotifyTriggerTime));
						return;
					}
				}

				// Handle case of being a preview for an AnimSequence
				const float ElapsedTime = UAnimNotifyLibrary::GetCurrentAnimationNotifyStateTime(EventReference);
				AnimActor->GetSkeletalMeshComponent()->GetSingleNodeInstance()->SetPosition(ElapsedTime);
			}
		}
	default:
		{
			return;
		}
	}
}
#endif
