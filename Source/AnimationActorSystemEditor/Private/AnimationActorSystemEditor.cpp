// Copyright 2025 Aaron Kemner, All Rights reserved.


#include "AnimationActorSystemEditor.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

void FAnimationActorSystemEditorModule::StartupModule()
{
	IPluginManager& PluginManager = IPluginManager::Get();

	const FString PluginName = "AnimationActorSystem";
	const TSharedPtr<IPlugin> ThisPlugin = PluginManager.FindPlugin(FStringView(PluginName));
	check(ThisPlugin)

	const FString ResourcesPath = ThisPlugin->GetBaseDir() / "Resources";
	if(!FPaths::DirectoryExists(ResourcesPath))
		return; // No clue why anyone would get this plugin but not get the resources dir...anyway, idrc 
	
	Style = MakeShareable(new FSlateStyleSet("AnimActorSystem_StyleSet"));
	Style->SetContentRoot(ResourcesPath);

	Style->Set("ClassThumbnail.AnimNotifyState_SpawnActorBase",
		new FSlateImageBrush( Style->RootToContentDir( TEXT("Icon_64"), TEXT(".png")),
			FVector2D(64.f, 64.f)));
	Style->Set("ClassIcon.AnimNotifyState_SpawnActorBase",
		new FSlateImageBrush( Style->RootToContentDir( TEXT("Icon_16"), TEXT(".png") ),
			FVector2D(16.0f, 16.0f)));
	
	FSlateStyleRegistry::RegisterSlateStyle(*Style);
}

void FAnimationActorSystemEditorModule::ShutdownModule()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(Style->GetStyleSetName());
}

IMPLEMENT_MODULE(FAnimationActorSystemEditorModule, AnimationActorSystemV2Editor)