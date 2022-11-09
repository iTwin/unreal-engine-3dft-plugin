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
                                                                 "JsonUtilities", "MeshDescription", "HTTP", "RHI", "RenderCore", "Projects" });
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
        }

        DynamicallyLoadedModuleNames.AddRange(new string[] { });

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/iModelDecoder/"));
            string PlatformDir = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "x86";
            string LibraryPath = Path.Combine(ThirdPartyPath, PlatformDir);

            // Console.WriteLine("\n\n... LibraryPath -> " + LibraryPath);

            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "includes"));

            RuntimeDependencies.Add(Path.Combine(LibraryPath, "decoder.dll"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/iModelDecoder/"));
            string LibraryPath = Path.Combine(ThirdPartyPath, "Mac");

            PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "libdecoder.a"));
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "includes"));
        }
    }
}
