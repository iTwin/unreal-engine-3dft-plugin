/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iModel3d.h"

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

#include "MeshComponentManager/MeshComponentManager.h"
#include "MeshComponentManager/GraphicOptions.h"

AiModel3d::AiModel3d(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	GraphicOptions = MakeShared<FGraphicOptions>();
	
	OpaqueMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/iModel3d/Materials/SolidMaterial.SolidMaterial'")));
	TranslucentMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/iModel3d/Materials/TranslucentMat.TranslucentMat'")));
}

void AiModel3d::Initialize(FString Url)
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

	MeshComponentManager->SetOptions(
		{ RequestsInParallel, MaxTrianglesPerBatch, { uint8_t(NearRangeGeometryQuality), uint8_t(FarRangeGeometryQuality)}, IgnoreTranslucency },
		{ ObjectLoadingSpeed, ShadowDistanceCulling, bUseDiskCache });

	MeshComponentManager->SetUrl(Url);
	for (const auto& Element : ElementInfos)
	{
		GraphicOptions->SetElement(Element);
	}

	MeshComponentManager->SetGraphicOptions(GraphicOptions);


	auto ParameterCollection = Cast<UMaterialParameterCollection>(StaticLoadObject(UMaterialParameterCollection::StaticClass(), nullptr, TEXT("MaterialParameterCollection'/iModel3d/Materials/iModelMaterialParameterCollection.iModelMaterialParameterCollection'")));
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

	bInitialized = true;
}

void AiModel3d::Deinitialize()
{
	if (!bInitialized)
	{
		return;
	}

	MeshComponentManager.Reset();

	bInitialized = false;
}

void AiModel3d::PostLoad()
{
	Super::PostLoad();
	if (LoadingMethod == ELoadingMethod::LM_Automatic && !ExportId.IsEmpty())
	{
		LoadModel(ExportId);
	}
}

#if WITH_EDITOR
void AiModel3d::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	if (PropertyName == "ExportId")
	{
		UE_LOG(LogTemp, Display, TEXT("ExportId modified"));
	}
	Super::PostEditChangeProperty(e);
}
#endif

void AiModel3d::BeginDestroy()
{
	Deinitialize();

	Super::BeginDestroy();
}

void AiModel3d::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bInitialized)
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

bool AiModel3d::IsInEditor() const
{
	return (GetWorld() && GetWorld()->WorldType == EWorldType::Editor);
}

bool AiModel3d::ShouldTickIfViewportsOnly() const
{
	return IsInEditor();
}

void AiModel3d::GetAnimationInfo(int32& Start, int32& End, int32& Duration, bool& Loaded)
{
	MeshComponentManager->GetAnimationInfo(Start, End, Duration, Loaded);
}

void AiModel3d::SetPlaybackPosition(int32 Time)
{
	MeshComponentManager->SetPlaybackPosition(Time);
}

void AiModel3d::GetStatusInfo(FString& Status, float& Percentage)
{
	MeshComponentManager->GetStatusInfo(Status, Percentage);
}

FString AiModel3d::GetElementId(uint32_t ElementIndex)
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

void AiModel3d::LoadModel(FString InExportId)
{
	FITwinServices::GetExportAndRefresh(InExportId, CancelRequest, [this](FITwinServices::FExportInfo ExportInfo, bool bRefreshUrl)
	{
		if (bRefreshUrl)
		{
			MeshComponentManager->SetUrl(ExportInfo.MeshUrl);
		}
		else
		{
			Deinitialize();
			Initialize(ExportInfo.MeshUrl);
		}
	});
}

void AiModel3d::Reset()
{
	Deinitialize();
}

/* To be implemented in the future

void AiModel3d::SetElementVisible(FString ElementId, bool bVisible)
{
	GraphicOptions->SetElementVisible(ElementId, bVisible);
	bDirtyOptions = true;
}

void AiModel3d::SetElementOffset(FString ElementId, FVector Offset)
{
	GraphicOptions->SetElementOffset(ElementId, Offset);
	bDirtyOptions = true;
}

void AiModel3d::SetElementMaterial(FString ElementId, FColor Color, float Specular, float Roughness, float Metalic)
{
	GraphicOptions->SetElementMaterial(ElementId, Color, Specular, Roughness, Metalic);
	bDirtyOptions = true;
}
*/
