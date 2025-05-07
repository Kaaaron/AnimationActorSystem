// Copyright 2025 Aaron Kemner, All Rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAnimationActorSystemEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
private:
    TSharedPtr<FSlateStyleSet> Style;
};
