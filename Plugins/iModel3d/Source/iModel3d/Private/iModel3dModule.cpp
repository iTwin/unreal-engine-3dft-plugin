// Copyright Epic Games, Inc. All Rights Reserved.

#include "iModel3dModule.h"

#define LOCTEXT_NAMESPACE "FiModel3dModule"

void FiModel3dModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FiModel3dModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FiModel3dModule, iModel3d)