#pragma once

#include "CoreMinimal.h"

#include "ElementInfo.h"

struct FMatOverride
{
	uint32_t Color;
	bool Enabled;
	float Specular;
	float Roughness;
	float Metalic;
	uint32_t OutColor;
};

struct FMaterialOptions
{
	// Debug
	bool UseBatchIdForColor;
	bool UseTileIdForColor;
	bool UsePartIdForColor;
	bool UseElementIdForColor;
	bool ExagerateColor;

	TArray<FMatOverride> Overrides;

	bool OverrideMaterials = true;
	bool HideTranslucentMaterials = false;
	bool DebugRGB = false;

	FMatOverride DefaultMaterial;
};

struct FGraphicOptions
{
	void SetElementVisible(FString ElementId, bool bVisible);

	void SetElementOffset(FString ElementId, FVector Offset);

	// void SetElementMaterial(FString ElementId, FColor Color, float Specular, float Roughness, float Metalic);

	void SetElement(const FElementInfo& Info);

	TMap<uint64_t, FElementInfo> ElementInfos;
	FMaterialOptions Materials;

private:
	FElementInfo* GetElement(FString ElementId);
};
