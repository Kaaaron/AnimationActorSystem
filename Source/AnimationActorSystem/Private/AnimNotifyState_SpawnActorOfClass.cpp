// Copyright 2025 Aaron Kemner, All Rights reserved.


#include "AnimNotifyState_SpawnActorOfClass.h"

#if WITH_EDITOR
FString UAnimNotifyState_SpawnActorOfClass::GetNotifyName_Implementation() const
{
	static FString NoneString = FString(TEXT("None"));
	if (AttachBone != NAME_None)
	{
		return FString::Printf(TEXT("Spawn %s on %s"), ClassToSpawn.IsValid() ? *ClassToSpawn.GetAssetName() : *NoneString,
							   *AttachBone.ToString());
	}
	return FString::Printf(TEXT("Spawn %s"), ClassToSpawn.IsValid() ? *ClassToSpawn.GetAssetName() : *NoneString);
}
#endif
