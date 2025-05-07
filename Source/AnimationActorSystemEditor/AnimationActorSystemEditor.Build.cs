// Copyright 2025 Aaron Kemner, All Rights reserved.


using UnrealBuildTool;

public class AnimationActorSystemEditor : ModuleRules
{
    public AnimationActorSystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "AnimationActorSystem"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Projects"
            }
        );
    }
}