#include "GraphicOptions.h"

#include <iostream>

void FGraphicOptions::SetElementVisible(FString ElementId, bool bVisible)
{
	auto Element = GetElement(ElementId);
	Element->bVisible = bVisible;
}

void FGraphicOptions::SetElementOffset(FString ElementId, FVector Offset)
{
	auto Element = GetElement(ElementId);
	Element->Offset = Offset;
}

/*
void FGraphicOptions::SetElementMaterial(FString ElementId, FColor Color, float Specular, float Roughness, float Metalic)
{
	auto Element = GetElement(ElementId);
	Element->bSetMaterial = true;
	Element->Color = Color;
	Element->Specular = Specular;
	Element->Roughness = Roughness;
	Element->Metalic = Metalic;
}
*/

void FGraphicOptions::SetElement(const FElementInfo& Info)
{
	auto Element = GetElement(Info.Id);
	*Element = Info;
}

FElementInfo* FGraphicOptions::GetElement(FString ElementId)
{
	uint64_t id = 0;
	if (ElementId.StartsWith("0x"))
	{
		std::cout << *ElementId.RightChop(2) << std::endl;
		id = FCString::Strtoui64(*ElementId.RightChop(2), NULL, 16);
	}
	else
	{
		id = FCString::Strtoui64(*ElementId, NULL, 10);
	}
	if (!ElementInfos.Contains(id))
	{
		ElementInfos.Add(id, {});
	}

	return &ElementInfos[id];
}