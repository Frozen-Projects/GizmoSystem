// Some copyright should be here...

using System;
using System.IO;
using UnrealBuildTool;

public class GizmoSystem : ModuleRules
{
	public GizmoSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // We use this for Open62541 GNUC and CLANG related errors. Beacuse Unreal Engine doesn't use them.
        UndefinedIdentifierWarningLevel = WarningLevel.Off;
        bEnableExceptions = true;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty", "vGizmo3D"));

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
                "InteractiveToolsFramework",
				// ... add private dependencies that you statically link with here ...	
			}
			);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
