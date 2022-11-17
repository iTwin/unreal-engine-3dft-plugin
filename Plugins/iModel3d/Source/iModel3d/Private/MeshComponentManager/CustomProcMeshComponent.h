/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/MeshComponent.h"

#include "CustomProcMeshComponent.generated.h"

class FPrimitiveSceneProxy;

struct FCustomProcMeshSection
{
public:
	TArray<uint32> Indices;
	TArray<FStaticMeshBuildVertex> Vertices;
	FBox SectionLocalBox;
	bool bSectionVisible;
};

UCLASS(hidecategories = (Object, LOD), meta = (BlueprintSpawnableComponent), ClassGroup = Rendering)
class UCustomProcMeshComponent : public UMeshComponent
{
	GENERATED_BODY()
public:

	UCustomProcMeshComponent(const FObjectInitializer& ObjectInitializer);

	void ClearAllMeshSections();

	void SetMeshSectionVisible(int32 SectionIndex, bool bNewVisibility);

	bool IsMeshSectionVisible(int32 SectionIndex) const;

	int32 GetNumSections() const;

	void SetProcMeshSection(int32 SectionIndex, const TSharedPtr<FCustomProcMeshSection>& Section);

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual int32 GetNumMaterials() const override;
	virtual void PostLoad() override;

private:
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	void UpdateLocalBounds();

	TArray<TSharedPtr<FCustomProcMeshSection>> ProcMeshSections;
	FBoxSphereBounds LocalBounds;

	friend class FProceduralMeshSceneProxy;
};


