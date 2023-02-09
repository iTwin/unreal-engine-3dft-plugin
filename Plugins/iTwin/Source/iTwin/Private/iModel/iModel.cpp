/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iModel/iModel.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameMode.h"

#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

#if WITH_EDITOR
#include "Editor/UnrealEd/Public/Editor.h"
#include "Editor/UnrealEd/Public/EditorViewportClient.h"
#endif

#include "iTwinPlatform/iTwinServices.h"
#include "MeshComponentManager/MeshComponentManager.h"
#include "MeshComponentManager/GraphicOptions.h"

AiModel::AiModel(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	GraphicOptions = MakeShared<FGraphicOptions>();
	
	OpaqueMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/iModel/Picking/SolidMaterial-Picking.SolidMaterial-Picking'")));
	TranslucentMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/iModel/Picking/TranslucentMat-Picking.TranslucentMat-Picking'")));
	
	CancelRequest = std::make_shared<FCancelRequest>();
}

void AiModel::InitializeMeshComponentManager()
{
	TArray<FMatOverride> Overrides;
	for (const auto& Mat : MaterialOverrides)
	{
		Overrides.Push({ Mat.Color.DWColor(), Mat.Enabled, Mat.Specular, Mat.Roughness, Mat.Metalic, Mat.OutColor.DWColor() });
	}
	const auto& Mat = DefaultMaterial;
	FMatOverride DefaultOverride = { Mat.Color.DWColor(), Mat.Enabled, Mat.Specular, Mat.Roughness, Mat.Metalic, Mat.OutColor.DWColor() };

	GraphicOptions->Materials = { Overrides, OverrideMaterials, HideTranslucentMaterials, DebugRGB, DefaultOverride };

	MeshComponentManager = MakeShared<FMeshComponentManager>(this, IsInEditor());

	MeshComponentManager->SetMaterials(OpaqueMaterial, TranslucentMaterial);

	MeshComponentManager->OnLoaded([this](bool bSuccess)
	{
		this->OniModelLoaded.Broadcast(bSuccess);
	});
	
	MeshComponentManager->SetOptions(
		{ RequestsInParallel, MaxTrianglesPerBatch, { uint8_t(NearRangeGeometryQuality), uint8_t(FarRangeGeometryQuality)}, IgnoreTranslucency },
		{ ObjectLoadingSpeed, ShadowDistanceCulling, bUseDiskCache });

	for (const auto& Element : ElementInfos)
	{
		GraphicOptions->SetElement(Element);
	}

	MeshComponentManager->SetGraphicOptions(GraphicOptions);


	auto ParameterCollection = Cast<UMaterialParameterCollection>(StaticLoadObject(UMaterialParameterCollection::StaticClass(), nullptr, TEXT("MaterialParameterCollection'/iModel/Materials/iModelMaterialParameterCollection.iModelMaterialParameterCollection'")));
	if (ParameterCollection)
	{
		auto Collection = GetWorld()->GetParameterCollectionInstance(ParameterCollection);
		Collection->SetScalarParameterValue("DebugRGB", GraphicOptions->Materials.DebugRGB ? 1 : 0);
	}

	bDirtyOptions = false;

#if WITH_EDITOR
	if (IsInEditor())
	{
		RootComponent->SetIsVisualizationComponent(true);
	}
#endif
}

void AiModel::DeinitializeMeshComponentManager()
{
	if (bMeshComponentManagerInitialized)
	{
		MeshComponentManager.Reset();
		bMeshComponentManagerInitialized = false;
	}
}

#if WITH_EDITOR
void AiModel::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	UE_LOG(LogTemp, Warning, TEXT("AiModel::PostEditChangeProperty()"));
	Super::PostEditChangeProperty(e);
	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	if (PropertyName == "iModelId" || PropertyName == "ChangesetId" || PropertyName == "ExportId")
	{
		if (LoadingMethod == ELoadingMethod::LM_Manual && !ExportId.IsEmpty())
		{
			LoadModel(ExportId);
		}
		else if (LoadingMethod == ELoadingMethod::LM_Automatic && !iModelId.IsEmpty() && !ChangesetId.IsEmpty())
		{
			LoadiModelChangeset();
		}
	}
}
#endif

void AiModel::BeginDestroy()
{
	DeinitializeMeshComponentManager();

	Super::BeginDestroy();
}

void AiModel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bTickCalled)
	{
		bTickCalled = true;
		if (LoadingMethod == ELoadingMethod::LM_Manual && !ExportId.IsEmpty())
		{
			LoadModel(ExportId);
		}
		else if (LoadingMethod == ELoadingMethod::LM_Automatic && !iModelId.IsEmpty() && !ChangesetId.IsEmpty())
		{
			LoadiModelChangeset();
		}
	}

	if (!bMeshComponentManagerInitialized)
	{
		return;
	}

	FVector CamLocation;
	FVector CamForward;
	auto FirstPlayerController = GetWorld()->GetFirstPlayerController();
	if (IsInEditor())
	{
#if WITH_EDITOR
		auto ViewportClient = reinterpret_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
		CamLocation = ViewportClient->GetViewLocation() - GetActorLocation();
		CamForward = ViewportClient->GetViewRotation().Vector();
#endif
	}
	else if (FirstPlayerController)
	{
		APlayerCameraManager* CamManager = FirstPlayerController->PlayerCameraManager;
		CamLocation = CamManager->GetCameraLocation() - GetActorLocation();
		CamForward = CamManager->GetCameraRotation().Vector();
		if (CamManager->GetCameraCacheTime() == 0)
		{
			return;
		}
	}

	if (bDirtyOptions)
	{
		MeshComponentManager->SetGraphicOptions(GraphicOptions);
		bDirtyOptions = false;
	}

	MeshComponentManager->Update(DeltaTime, CamLocation, CamForward);
}

bool AiModel::IsInEditor() const
{
	return (GetWorld() && GetWorld()->WorldType == EWorldType::Editor);
}

bool AiModel::ShouldTickIfViewportsOnly() const
{
	return IsInEditor();
}

void AiModel::GetAnimationInfo(int32& Start, int32& End, int32& Duration, bool& Loaded)
{
	MeshComponentManager->GetAnimationInfo(Start, End, Duration, Loaded);
}

void AiModel::SetPlaybackPosition(int32 Time)
{
	MeshComponentManager->SetPlaybackPosition(Time);
}

void AiModel::GetStatusInfo(FString& Status, float& Percentage)
{
	MeshComponentManager->GetStatusInfo(Status, Percentage);
}

void AiModel::GetModel3DInfo(FiModel3DInfo& Info)
{
	Info = MeshComponentManager->GetModel3DInfo();
}


FString AiModel::GetElementId(uint32_t ElementIndex)
{
	if (!MeshComponentManager)
	{
		return FString("");
	}
	else
	{
		auto ElementId = MeshComponentManager->GetElementId(ElementIndex);
		if (ElementId == uint64_t(-1))
		{
			return FString("");
		}
		if ((ElementId >> 32) > 0)
		{
			return FString::Printf(TEXT("0x%x%08x"), (ElementId >> 32), ElementId & 0xffffffff);
		}
		else
		{
			return FString::Printf(TEXT("0x%x"), ElementId);
		}
	}
}

void AiModel::LoadModel(FString InExportId)
{
	FITwinServices::GetExportAndRefresh(InExportId, *CancelRequest, [this](FITwinServices::FExportInfo ExportInfo, bool bRefreshUrl)
	{
		if (bRefreshUrl)
		{
			MeshComponentManager->SetUrl(ExportInfo.MeshUrl);
		}
		else if (ExportInfo.Status == "Complete")
		{
			DeinitializeMeshComponentManager();
			InitializeMeshComponentManager();
			bMeshComponentManagerInitialized = true;

			MeshComponentManager->SetUrl(ExportInfo.MeshUrl);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("iModel is being exported! Try re-loading the iModel in a few seconds."));
		}
	});
}

void AiModel::Reset()
{
	DeinitializeMeshComponentManager();
}

void AiModel::LoadiModelChangeset()
{
	FITwinServices::AutoExportAndLoad(*CancelRequest, iModelId, ChangesetId, [this](auto ExportId)
	{
		if (ExportId == "")
		{
			UE_LOG(LogTemp, Error, TEXT("Processing in progress!"));
		}
		else
		{
			#if WITH_EDITOR
			if (IsInEditor())
			{
				this->ExportId = ExportId;
			}
			#endif
			return LoadModel(ExportId);
		}
	});
}

/* To be implemented in the future

void AiModel::SetElementVisible(FString ElementId, bool bVisible)
{
	GraphicOptions->SetElementVisible(ElementId, bVisible);
	bDirtyOptions = true;
}

void AiModel::SetElementOffset(FString ElementId, FVector Offset)
{
	GraphicOptions->SetElementOffset(ElementId, Offset);
	bDirtyOptions = true;
}

void AiModel::SetElementMaterial(FString ElementId, FColor Color, float Specular, float Roughness, float Metalic)
{
	GraphicOptions->SetElementMaterial(ElementId, Color, Specular, Roughness, Metalic);
	bDirtyOptions = true;
}
*/
