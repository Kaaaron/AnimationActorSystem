// Copyright 2025 Aaron Kemner, All Rights reserved.


#include "AnimNotifyState_SpawnStaticMesh.h"

void UAnimNotifyState_SpawnStaticMesh::PostSpawnActor(AActor* SpawnedActor, UAnimationActorSubsystem* Subsystem,
                                                      USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                                      const FAnimNotifyEventReference& EventReference)
{
	Super::PostSpawnActor(SpawnedActor, Subsystem, MeshComp, Animation, TotalDuration, EventReference);

	const AStaticMeshActor* SKMA = CastChecked<AStaticMeshActor>(SpawnedActor);
	UStaticMeshComponent* Comp = SKMA->GetStaticMeshComponent();
	check(Comp)
	Comp->SetStaticMesh(MeshToSpawn);
	if(bOverrideCollisionProfile)
	{
		Comp->SetCollisionProfileName(CollisionProfileOverride.Name, true);
	}

	Comp->SetCanEverAffectNavigation(UAnimationActorSystemSettings::Get()->bStaticCanAffectNavigation);
}

FString UAnimNotifyState_SpawnStaticMesh::GetNotifyName_Implementation() const
{
	return BuildNotifyNameFromObject(MeshToSpawn);
}
