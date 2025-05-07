// Copyright 2025 Aaron Kemner, All Rights reserved.


#include "AnimationActorSystemSettings.h"

#include "AnimationActorSystem.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#if WITH_EDITOR	
void UAnimationActorSystemSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(!SkeletalMeshActorClass)
	{
		UE_LOG(LogAnimActorSys, Error, TEXT("SkeletalMeshActorClass is invalid. Restoring default value."))
		SkeletalMeshActorClass = ASkeletalMeshActor::StaticClass();
	}
	
	if(!StaticMeshActorClass)
	{
		UE_LOG(LogAnimActorSys, Error, TEXT("StaticMeshActorClass is invalid. Restoring default value."))
		StaticMeshActorClass = AStaticMeshActor::StaticClass();
	}
	
	if(PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_CHECKED(UAnimationActorSystemSettings, ActorClassLoadingBehaviour))
	{

		if(ActorClassLoadingBehaviour == EAnimActorClassLoadingBehaviour::BeginPlay_Async
			|| ActorClassLoadingBehaviour == EAnimActorClassLoadingBehaviour::BeginPlay_Blocking)
		{
			const FString Message = FString(TEXT("Actor Class Loading Behaviour is not allowed to be BeginPlay_[...]. "
						"Switching to corresponding FirstTimeRequested_[...] Value"));
			UE_LOG(LogAnimActorSys, Error, TEXT("%s"), *Message)
#if WITH_EDITOR
			FSlateNotificationManager& SNM = FSlateNotificationManager::Get();
			FNotificationInfo Notification = FNotificationInfo(FText::FromString(Message));
			Notification.ExpireDuration = 5.f;
			SNM.AddNotification(Notification);
#endif
			
			switch (ActorClassLoadingBehaviour)
			{
			case EAnimActorClassLoadingBehaviour::BeginPlay_Async:
				ActorClassLoadingBehaviour = EAnimActorClassLoadingBehaviour::FirstTimeRequested_Async;
				break;
			case EAnimActorClassLoadingBehaviour::BeginPlay_Blocking:
				ActorClassLoadingBehaviour = EAnimActorClassLoadingBehaviour::FirstTimeRequested_Blocking;
				break;
			default:
				break;
			}
		}
	}
}
#endif

