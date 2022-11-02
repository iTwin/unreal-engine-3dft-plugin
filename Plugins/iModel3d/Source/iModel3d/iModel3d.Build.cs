// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;

public class iModel3d : ModuleRules
{
    public iModel3d(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp17;

        PublicIncludePaths.AddRange(new string[] { });

        PrivateIncludePaths.AddRange(new string[] { });

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "ProceduralMeshComponent" });

        PrivateDependencyModuleNames.AddRange(new string[] { "CoreUObject", "Engine", "Slate", "SlateCore", "WebSockets", "Json",
                                                                 "JsonUtilities", "MeshDescription", "HTTP", "RHI", "RenderCore" });
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
        }

        DynamicallyLoadedModuleNames.AddRange(new string[] { });

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/iModelDecoder/"));

            string LibraryPath = Path.Combine(ThirdPartyPath, (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "x86");

            // Console.WriteLine("\n\n... LibrariesPath -> " + LibraryPath);
            // Console.WriteLine("\n\n... Includes -> " + ThirdPartyPath);

            PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "decoder.lib"));
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "includes"));


            // Definitions.Add(string.Format("WITH_YJ_MAGIC_LIB_BINDING={0}", isLibrarySupported ? 1 : 0));
        } else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/iModelDecoder/"));
            string LibraryPath = Path.Combine(ThirdPartyPath, "Mac");

            PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "libdecoder.a"));
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "includes"));
        }
    }
}
