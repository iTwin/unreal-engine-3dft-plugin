/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MaterialOverride.h"
#include "ElementInfo.h"

#include "iModel.generated.h"

class FMeshComponentManager;
struct FGraphicOptions;
struct FCancelRequest;

UENUM(BlueprintType)
enum class EGeometryQuality : uint8
{
	GQ_Maximum UMETA(DisplayName = "High"),
	GQ_High UMETA(DisplayName = "Normal")
};

UENUM(BlueprintType)
enum class ELoadingMethod : uint8
{
	LM_Automatic UMETA(DisplayName = "Automatic"),
	LM_Manual UMETA(DisplayName = "Manual")
};


UCLASS()
class ITWIN_API AiModel : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = " Model|Animation")
		void GetAnimationInfo(int32& Start, int32& End, int32& Duration, bool& Loaded);

	UFUNCTION(BlueprintCallable, Category = "_iModel|Animation")
		void SetPlaybackPosition(int32 Time);

	UFUNCTION(BlueprintCallable, Category = "&Model|Status")
		void GetStatusInfo(FString& Status, float& Percentage);

	UFUNCTION(BlueprintCallable, Category = ".Model|Load")
		void LoadModel(FString ExportId);

	UFUNCTION(BlueprintCallable, Category = "-Model|Load")
		void Reset();

	/* To be implemented in the future

	UFUNCTION(BlueprintCallable, Category = "Model|Elements")
		void SetElementVisible(FString ElementId, bool bVisible = true);

	UFUNCTION(BlueprintCallable, Category = "Model|Elements")
		void SetElementOffset(FString ElementId, FVector Offset = FVector(0, 0, 0));

	UFUNCTION(BlueprintCallable, Category = "Model|Elements")
		void SetElementMaterial(FString ElementId, FColor Color = FColor(255, 255, 0), float Specular = 0.0, float Roughness = 1.0, float Metalic = 0.0);
	*/


private:
	UPROPERTY(EditAnywhere, Category = "Model|Loading")
		ELoadingMethod LoadingMethod = ELoadingMethod::LM_Manual;

	UPROPERTY(EditAnywhere, Category = "Model|Loading", meta = (EditCondition = "LoadingMethod == ELoadingMethod::LM_Manual"))
		FString ExportId = "";

	UPROPERTY(EditAnywhere, Category = "Model|Loading", meta = (EditCondition = "LoadingMethod == ELoadingMethod::LM_Automatic", DisplayName = "iModel Id"))
		FString iModelId = "";

	UPROPERTY(EditAnywhere, Category = "Model|Loading", meta = (EditCondition = "LoadingMethod == ELoadingMethod::LM_Automatic"))
		FString ChangesetId = "";

	UPROPERTY(EditAnywhere, Category = "Model|Loading")
		float ObjectLoadingSpeed = 4.f;

	UPROPERTY(EditAnywhere, Category = "Model|Loading")
		uint32 RequestsInParallel = 12;

	UPROPERTY(EditAnywhere, Category = "Model|Loading")
		bool bUseDiskCache = true;

	UPROPERTY(EditAnywhere, Category = "Model|Render Materials")
		UMaterial* OpaqueMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Model|Render Materials")
		UMaterial* TranslucentMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "Model|Optimization")
		uint32 MaxTrianglesPerBatch = 50000;

	UPROPERTY(EditAnywhere, Category = "Model|Geometry Quality", DisplayName = "Near Reange")
		EGeometryQuality NearRangeGeometryQuality = EGeometryQuality::GQ_High;

	UPROPERTY(EditAnywhere, Category = "Model|Geometry Quality", DisplayName = "Far Range")
		EGeometryQuality FarRangeGeometryQuality = EGeometryQuality::GQ_High;

	UPROPERTY(EditAnywhere, Category = "Model|Optimization")
		float ShadowDistanceCulling = 15000.f;

	UPROPERTY(EditAnywhere, Category = "Model|Elements")
		TArray<FElementInfo> ElementInfos;

	UPROPERTY(EditAnywhere, Category = "Model|Material Overrides")
		TArray<FMaterialOverride> MaterialOverrides;

	UPROPERTY(EditAnywhere, Category = "Model|Material Overrides")
		bool OverrideMaterials = true;

	UPROPERTY(EditAnywhere, Category = "Model|Material Overrides")
		bool HideTranslucentMaterials = false;

	UPROPERTY(EditAnywhere, Category = "Model|Material Overrides")
		bool IgnoreTranslucency = false;

	UPROPERTY(EditAnywhere, Category = "Model|Material Overrides")
		bool DebugRGB = false;

	UPROPERTY(EditAnywhere, Category = "Model|Material Overrides")
		FMaterialOverride DefaultMaterial = { { FColor(0xffffffff) }, "Default", false, 0, 0, 0, FColor(0xffffffff) };

protected:
	virtual void BeginDestroy() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual bool ShouldTickIfViewportsOnly() const override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FString GetElementId(uint32_t ElementIndex);

private:
	TSharedPtr<FGraphicOptions> GraphicOptions;
	bool bDirtyOptions = false;

	TSharedPtr<FMeshComponentManager> MeshComponentManager;
	bool bMeshComponentManagerInitialized = false;
	bool bTickCalled = false;

	std::shared_ptr<FCancelRequest> CancelRequest;

	bool IsInEditor() const;
	void InitializeMeshComponentManager();
	void DeinitializeMeshComponentManager();
	void LoadiModelChangeset();

};
