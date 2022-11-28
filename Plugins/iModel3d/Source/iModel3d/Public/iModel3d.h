/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MaterialOverride.h"
#include "ElementInfo.h"
#include "iTwinPlatform/iTwinServices.h"

#include "iModel3d.generated.h"

class FMeshComponentManager;
struct FGraphicOptions;

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
class IMODEL3D_API AiModel3d : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "iModel|Animation")
		void GetAnimationInfo(int32& Start, int32& End, int32& Duration, bool& Loaded);

	UFUNCTION(BlueprintCallable, Category = "iModel|Animation")
		void SetPlaybackPosition(int32 Time);

	UFUNCTION(BlueprintCallable, Category = "iModel|Status")
		void GetStatusInfo(FString& Status, float& Percentage);

	UFUNCTION(BlueprintCallable, Category = "iModel|Load")
		void LoadModel(FString ExportId);

	UFUNCTION(BlueprintCallable, Category = "iModel|Load")
		void Reset();

	/* To be implemented in the future

	UFUNCTION(BlueprintCallable, Category = "iModel|Elements")
		void SetElementVisible(FString ElementId, bool bVisible = true);

	UFUNCTION(BlueprintCallable, Category = "iModel|Elements")
		void SetElementOffset(FString ElementId, FVector Offset = FVector(0, 0, 0));

	UFUNCTION(BlueprintCallable, Category = "iModel|Elements")
		void SetElementMaterial(FString ElementId, FColor Color = FColor(255, 255, 0), float Specular = 0.0, float Roughness = 1.0, float Metalic = 0.0);
	*/


private:
	UPROPERTY(EditAnywhere, Category = "iModel|Loading")
		ELoadingMethod LoadingMethod = ELoadingMethod::LM_Automatic;

	UPROPERTY(EditAnywhere, Category = "iModel|Loading", meta = (EditCondition = "LoadingMethod == ELoadingMethod::LM_Automatic"))
		FString ExportId = "69528456-7b4e-4de1-8049-4777cbefd201";

	UPROPERTY(EditAnywhere, Category = "iModel|Loading")
		float ObjectLoadingSpeed = 1.f;

	UPROPERTY(EditAnywhere, Category = "iModel|Loading")
		uint32 RequestsInParallel = 4;

	UPROPERTY(EditAnywhere, Category = "iModel|Loading")
		bool bUseDiskCache = true;

	UPROPERTY(EditAnywhere, Category = "iModel|Render Materials")
		UMaterial* OpaqueMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "iModel|Render Materials")
		UMaterial* TranslucentMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "iModel|Optimization")
		uint32 MaxTrianglesPerBatch = 25000;

	UPROPERTY(EditAnywhere, Category = "iModel|Geometry Quality", DisplayName = "Near Reange")
		EGeometryQuality NearRangeGeometryQuality = EGeometryQuality::GQ_High;

	UPROPERTY(EditAnywhere, Category = "iModel|Geometry Quality", DisplayName = "Far Range")
		EGeometryQuality FarRangeGeometryQuality = EGeometryQuality::GQ_High;

	UPROPERTY(EditAnywhere, Category = "iModel|Optimization")
		float ShadowDistanceCulling = 15000.f;

	UPROPERTY(EditAnywhere, Category = "iModel|Elements")
		TArray<FElementInfo> ElementInfos;

	UPROPERTY(EditAnywhere, Category = "iModel|Material Overrides")
		TArray<FMaterialOverride> MaterialOverrides;

	UPROPERTY(EditAnywhere, Category = "iModel|Material Overrides")
		bool OverrideMaterials = true;

	UPROPERTY(EditAnywhere, Category = "iModel|Material Overrides")
		bool HideTranslucentMaterials = false;

	UPROPERTY(EditAnywhere, Category = "iModel|Material Overrides")
		bool IgnoreTranslucency = false;

	UPROPERTY(EditAnywhere, Category = "iModel|Material Overrides")
		bool DebugRGB = false;

	UPROPERTY(EditAnywhere, Category = "iModel|Material Overrides")
		FMaterialOverride DefaultMaterial = { { FColor(0xffffffff) }, "Default", false, 0, 0, 0, FColor(0xffffffff) };

protected:
	virtual void BeginDestroy() override;
	virtual void PostLoad() override;
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
	bool bInitialized = false;

	FITwinServices::FCancelRequest CancelRequest;

	bool IsInEditor() const;
	void Initialize(FString Url);
	void Deinitialize();
};
