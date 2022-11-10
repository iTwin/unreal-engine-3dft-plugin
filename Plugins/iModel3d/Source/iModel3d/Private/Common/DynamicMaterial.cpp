#include "DynamicMaterial.h"

void FDynamicMaterial::Initialize(UMaterialInterface* Material)
{
	BaseMaterial = Material;
}

UMaterialInstanceDynamic* FDynamicMaterial::Get()
{
	if (!Instance || !Instance->IsValidLowLevel())
	{
		if (BaseMaterial && BaseMaterial->IsValidLowLevel())
		{
			static TAtomic<int> MaterialId = 0;
			const FName SlotName(*(TEXT("iModelMaterial") + FString::FromInt(MaterialId++)));

			Instance = UMaterialInstanceDynamic::Create(BaseMaterial, nullptr, SlotName);
			Instance->SetFlags(RF_Transient | RF_DuplicateTransient | RF_TextExportTransient);
		}
	}
	return Instance;
}
