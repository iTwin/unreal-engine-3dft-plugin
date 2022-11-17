/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "CreateIModelDecoder.h"

#include "GenericPlatform/GenericPlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

#if PLATFORM_WINDOWS

std::shared_ptr<IModelDecoder> CreateIModelDecoder()
{
	const FString BasePluginDir = IPluginManager::Get().FindPlugin("iModel3d")->GetBaseDir();
	const FString DllPath = FPaths::Combine(*BasePluginDir, TEXT("ThirdParty/iModelDecoder/windows/x64/decoder.dll"));

	auto dll = FPlatformProcess::GetDllHandle(*DllPath);
	if (!dll)
	{
		return nullptr;
	}

	return std::shared_ptr<IModelDecoder>(CreateDecoder(), [dll](IModelDecoder* Decoder) {
		Decoder->Release();
		FPlatformProcess::FreeDllHandle(dll);
	});
}

#else

std::shared_ptr<IModelDecoder> CreateIModelDecoder()
{
	return std::shared_ptr<IModelDecoder>(CreateDecoder(), [](IModelDecoder* Decoder) { Decoder->Release(); });
}

#endif
