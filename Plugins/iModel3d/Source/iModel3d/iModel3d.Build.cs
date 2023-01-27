/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
                                                                 "JsonUtilities", "MeshDescription", "HTTP", "HTTPServer", "RHI", "RenderCore", "Projects" });
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd" });
        }

        DynamicallyLoadedModuleNames.AddRange(new string[] { });

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/iModelDecoder/"));
            string WinPlatform = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "x86";
            string LibraryPath = Path.Combine(ThirdPartyPath, "windows", WinPlatform);

            // Console.WriteLine("\n\n... LibraryPath -> " + LibraryPath);

            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "includes"));

            PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "decoder.lib"));
            RuntimeDependencies.Add("$(PluginDir)/ThirdParty/iModelDecoder/windows/" + WinPlatform + "/decoder.dll");
            PublicDelayLoadDLLs.Add("decoder.dll");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/iModelDecoder/"));
            string LibraryPath = Path.Combine(ThirdPartyPath, "macos");

            PublicAdditionalLibraries.Add(Path.Combine(LibraryPath, "libdecoder.dylib"));
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "includes"));
        }
    }
}
