// Fill out your copyright notice in the Description page of Project Settings.

#include "iModel3d.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameMode.h"

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
}

void AiModel3d::Initialize()
{
	if (bInitialized)
	{
		return;
	}

	OpaqueMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/iModel3d/Materials/SolidMaterial.SolidMaterial'")));
	TranslucentMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("Material'/iModel3d/Materials/TranslucentMat.TranslucentMat'")));

	TArray<FMatOverride> Overrides;
	for (const auto& Mat : MaterialOverrides)
	{
		Overrides.Push({ Mat.Color.DWColor(), Mat.Enabled, Mat.Specular, Mat.Roughness, Mat.Metalic, Mat.OutColor.DWColor() });
	}
	const auto& Mat = DefaultMaterial;
	FMatOverride DefaultOverride = { Mat.Color.DWColor(), Mat.Enabled, Mat.Specular, Mat.Roughness, Mat.Metalic, Mat.OutColor.DWColor() };

	GraphicOptions->Materials = {
		UseBatchIdForColor, UseTileIdForColor, UsePartIdForColor, UseElementIdForColor, ExagerateColor, Overrides, OverrideMaterials, HideTranslucentMaterials, DebugRGB, DefaultOverride
	};

	MeshComponentManager = MakeShared<FMeshComponentManager>(this, IsInEditor());

	MeshComponentManager->SetMaterials(OpaqueMaterial, TranslucentMaterial);

	MeshComponentManager->SetOptions(
		{ ObjectLoadingSpeed, bShowObjectsWhileLoading, RequestsInParallel, MaxTrianglesPerBatch, ShadowDistanceCulling, { uint8_t(NearRangeGeometryQuality), uint8_t(FarRangeGeometryQuality)}, IgnoreTranslucency },
		PrintBatches, LocalPath, bUseDiskCache);

	for (const auto& Element : ElementInfos)
	{
		GraphicOptions->SetElement(Element);
	}

	MeshComponentManager->SetGraphicOptions(GraphicOptions);
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

void AiModel3d::BeginPlay()
{
	Super::BeginPlay();

	Initialize();
}

void AiModel3d::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Deinitialize();

	Super::EndPlay(EndPlayReason);
}

void AiModel3d::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector CamLocation;
	FVector CamForward;
	auto FirstPlayerController = GetWorld()->GetFirstPlayerController();
	if (IsInEditor())
	{
#if WITH_EDITOR
		/*
		if (!bShowInEditor)
		{
			if (MeshComponentManager)
			{
				MeshComponentManager->HideVisible();
			}
			return;
		}
		*/

		Initialize();

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

	if (PrintBatches)
	{
		GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor::Yellow, FString::Printf(TEXT("Drawn Batches: %i"), MeshComponentManager->GetNumDrawcalls()));
	}
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

void AiModel3d::LoadModel(FString Url)
{
	Deinitialize();
	LocalPath = Url;

	Initialize();
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
