#include "CreateIModelDecoder.h"

#include "GenericPlatform/GenericPlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

#if PLATFORM_WINDOWS
typedef IModelDecoder* (__cdecl* TCreateDecoderFn)();

std::shared_ptr<IModelDecoder> CreateIModelDecoder()
{
	const FString BasePluginDir = IPluginManager::Get().FindPlugin("iModel3d")->GetBaseDir();
	const FString DllPath = FPaths::Combine(*BasePluginDir, TEXT("ThirdParty/iModelDecoder/windows/x64/decoder.dll"));

	auto dll = FPlatformProcess::GetDllHandle(*DllPath);
	if (!dll)
	{
		return nullptr;
	}

	auto CreateDecoderFn = reinterpret_cast<TCreateDecoderFn>(FPlatformProcess::GetDllExport(dll, TEXT("CreateDecoder")));
	if (!CreateDecoderFn)
	{
		FPlatformProcess::FreeDllHandle(dll);
		return nullptr;
	}

	return std::shared_ptr<IModelDecoder>(CreateDecoderFn(), [dll](IModelDecoder* Decoder) {
		Decoder->Release();
		FPlatformProcess::FreeDllHandle(dll);
	});
}

#else

// Function exported in the static library
IModelDecoder* CreateDecoder();

std::shared_ptr<IModelDecoder> CreateIModelDecoder()
{
	return std::shared_ptr<IModelDecoder>(CreateDecoder(), [](IModelDecoder* Decoder) { Decoder->Release(); });
}

#endif
