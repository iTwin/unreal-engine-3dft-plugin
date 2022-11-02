#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "CustomProcMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Common/DynamicTexture.h"

#include "BatchLoaderRunnable.h"
#include "ProceduralMeshes.h"
#include "DataRequestManager.h"
#include "GraphicOptions.h"
#include "Animation/AnimationManager.h"

class FMeshComponentManager
{
public:
	FMeshComponentManager(AActor* InActor, bool bIsInEditor);
	~FMeshComponentManager();

	void AddBinaryAssets(const uint8* Data, size_t Size, FString RelativePath);

	void SetMaterials(UMaterialInterface* Opaque, UMaterialInterface* Translucent);

	void SetOptions(FIMOptions InOptions, bool PrintBatches, FString LocalPath, bool bUseDiskCache);

	void SetGraphicOptions(const TSharedPtr<FGraphicOptions>& GraphicOptions);

	int GetNumInstances();

	int GetNumDrawcalls();

	void Update(float DeltaTime, FVector CamLocation, FVector CamForward);

	/* Animation */
	void GetAnimationInfo(int32& Start, int32& End, int32& Duration, bool& Loaded);

	void SetPlaybackPosition(int32 Time);

	/* Status */
	void GetStatusInfo(FString& Status, float& Percentage);

	uint64_t GetElementId(uint32_t ElementIndex);


private:
	struct FBatchPMComponent
	{
		UCustomProcMeshComponent* MeshComponent;
		FTileMaterial Material;
		FTilePosition Position;
		bool bShadowsOn;
		int32 BatchId;
	};

	bool bShowBatchesWhileLoading = true;
	AActor* Actor;
	std::shared_ptr<FProceduralMeshes> ProceduralMeshes;
	struct
	{
		UMaterialInstanceDynamic* Opaque;
		UMaterialInstanceDynamic* Translucent;
	} Materials;

	FIMOptions Options;
	bool PrintBatches;
	std::shared_ptr<IModelDecoder> ModelDecoder;

	TArray<FBatchPMComponent> VisibleMeshes;
	TArray<FBatchPMComponent> LoadingMeshes;
	TSharedPtr<FProceduralTileMesh> CurrentRenderableBatch;
	std::unique_ptr<FBatchLoaderRunnable> BatchLoaderRunnable;
	FRunnableThread* BatchLoaderThread;

	FAnimationManager Animation;

	TSharedPtr<FDataRequestManager> DataRequests;

	void UpdateBatches(float DeltaTime, FVector CamLocation);
	void UpdateShadows(FVector CamLocation);
	void UpdateComponentShadows(FVector CamLocation, FBatchPMComponent& Component);
};
