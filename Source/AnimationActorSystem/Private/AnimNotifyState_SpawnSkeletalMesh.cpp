// Copyright 2025 Aaron Kemner, All Rights reserved.


#include "AnimNotifyState_SpawnSkeletalMesh.h"

void UAnimNotifyState_SpawnSkeletalMesh::PostSpawnActor(AActor* SpawnedActor, UAnimationActorSubsystem* Subsystem,
                                                        USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                                        const FAnimNotifyEventReference& EventReference)
{
	Super::PostSpawnActor(SpawnedActor, Subsystem, MeshComp, Animation, TotalDuration, EventReference);

	const ASkeletalMeshActor* SKMA = CastChecked<ASkeletalMeshActor>(SpawnedActor);
	USkeletalMeshComponent* Comp = SKMA->GetSkeletalMeshComponent();
	check(Comp)
	Comp->SetSkeletalMesh(MeshToSpawn);
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
