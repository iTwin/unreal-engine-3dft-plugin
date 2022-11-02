// Fill out your copyright notice in the Description page of Project Settings.


#include "Helpers/PickingActor.h"
#include "ProceduralMeshComponent.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/Classes/GameFramework/PlayerController.h"
#include "IModelElementIndex.h"

#define PICKING_LOGGING 1

// Sets default values
APickingActor::APickingActor()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	InitSceneCaptureComponent();
}

// Called when the game starts or when spawned
void APickingActor::BeginPlay()
{
	Super::BeginPlay();

	InitRenderTarget();
	// Optionally capture only specified components
#if 0
	SetCapturedComponents();
#endif
}

// Called every frame
void APickingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


/// Picking

void APickingActor::PickObjectAtMousePosition(FString& ElementId, FVector2D& MousePosition)
{
	// 1. Acquire mouse position
	GetWorld()->GetGameViewport()->GetMousePosition(MousePosition);

	// 2. Pick Object Id at mouse position
	auto SelectedColor = PickObjectAt(MousePosition);
	auto SelectedElement = IModelElementIndex(SelectedColor.X, SelectedColor.Y);
	FVector2f SelectedElementXY = SelectedElement;

#ifdef PICKING_LOGGING
	auto XY = SelectedElementXY * float(SelectedElement.scale());
	UE_LOG(LogTemp, Warning, TEXT("OnClick Selected Element Id: %u, %f, %f"), SelectedElement, XY.X, XY.Y);
#endif

	// 3. Set PickedObjectId material parameter to selected Object Id
	auto Collection = GetWorld()->GetParameterCollectionInstance(PickingMaterialParameterCollection);
	Collection->SetVectorParameterValue("PickedObjectId", FLinearColor(SelectedElementXY.X, SelectedElementXY.Y, 0.f, 0.f));

	ElementPickedEvent.Broadcast({});

	ElementId = iModel3d->GetElementId(SelectedElement);
}

void APickingActor::CaptureScene()
{
	// Switch material parameter collection to Object Id mode, take a scene capture, and switch back
	auto Collection = GetWorld()->GetParameterCollectionInstance(PickingMaterialParameterCollection);

	Collection->SetScalarParameterValue("PickingMode", 1.f);
	SceneCaptureComponent2D->CaptureScene();
	Collection->SetScalarParameterValue("PickingMode", 0.f);
}

EPixelFormat ReadRenderTargetHelper(
	TArray<FLinearColor>& OutHDRValues,
	UObject* WorldContextObject,
	UTextureRenderTarget2D* TextureRenderTarget,
	int32 X,
	int32 Y,
	int32 Width,
	int32 Height,
	bool bNormalize);

FVector2D APickingActor::PickObjectAt(const FVector2D& ScreenCoords)
{
	// 1. Make scene capture match main camera view
	FMinimalViewInfo MainCameraView;
	GetWorld()->GetFirstPlayerController()->PlayerCameraManager->CalcCamera(0.f, MainCameraView);
	SceneCaptureComponent2D->SetCameraView(MainCameraView);

	//FSceneViewProjectionData projData;
	//projData.SetViewRectangle(FIntRect(mousePos.X, mousePos.Y, mousePos.X + 1, mousePos.Y + 1));
	//FMinimalViewInfo::CalculateProjectionMatrixGivenView(view, EAspectRatioAxisConstraint::AspectRatio_MaintainXFOV, GetWorld()->GetGameViewport()->Viewport, projData);
	//SceneCaptureComponent2D->CustomProjectionMatrix = projData.ProjectionMatrix;
	//SceneCaptureComponent2D->bUseCustomProjectionMatrix = true;

	FVector2D ViewportSize;
	GetWorld()->GetGameViewport()->GetViewportSize(ViewportSize);

	// 2. Resize texture target if viewport size changed
	if (PickingTextureRenderTarget2D->SizeX != ViewportSize.X || PickingTextureRenderTarget2D->SizeY != ViewportSize.Y)
	{
		PickingTextureRenderTarget2D->ResizeTarget(ViewportSize.X, ViewportSize.Y);
	}

	// 3. Capture Scene to Picking Texture Render Target
	CaptureScene();

	// 4. Find texture coordinates matching mouse position
	FVector2D TexCoords = ScreenCoords / ViewportSize;

	// 5. Read back from render target at designated coordinates
	TArray<FLinearColor> OutHDRValues;
	auto OutFormat = ReadRenderTargetHelper(OutHDRValues, GetWorld(), PickingTextureRenderTarget2D,
		TexCoords.X * PickingTextureRenderTarget2D->SizeX, TexCoords.Y * PickingTextureRenderTarget2D->SizeY, 1, 1, false);
	if (OutFormat == PF_Unknown) {
		return FVector2D(0, 0);
	}

	FLinearColor PickedColor = OutHDRValues[0];
#ifdef PICKING_LOGGING
	UE_LOG(LogTemp, Warning, TEXT("PickObjectAt ScreenCoords: %f, %f;  TexCoords: %f, %f;  PickedColor: %f %f %f %f"),
		ScreenCoords.X, ScreenCoords.Y,
		TexCoords.X, TexCoords.Y,
		PickedColor.R, PickedColor.G, PickedColor.B, PickedColor.A);
#endif

	// 6. Find selected Object Id
	return FVector2D(PickedColor.R, PickedColor.G);
}


/// SceneCaptureComponent

void APickingActor::InitSceneCaptureComponent() {
	SceneCaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent2D"));
	SceneCaptureComponent2D->bCaptureEveryFrame = false;
	SceneCaptureComponent2D->bCaptureOnMovement = false;

	SceneCaptureComponent2D->ShowFlags.SetLighting(false);
	SceneCaptureComponent2D->ShowFlags.SetPostProcessing(false);
	SceneCaptureComponent2D->ShowFlags.SetColorGrading(false);
	SceneCaptureComponent2D->ShowFlags.SetTonemapper(false);
	SceneCaptureComponent2D->ShowFlags.SetAtmosphere(false);
	SceneCaptureComponent2D->ShowFlags.SetFog(false);
	SceneCaptureComponent2D->ShowFlags.SetAntiAliasing(false);

	SceneCaptureComponent2D->PostProcessSettings.bOverride_AutoExposureBias = true;
	SceneCaptureComponent2D->PostProcessSettings.AutoExposureBias = 0;

	SceneCaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
}

void APickingActor::InitRenderTarget() {
	PickingTextureRenderTarget2D->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA32f;
	PickingTextureRenderTarget2D->Filter = TextureFilter::TF_Nearest;
	PickingTextureRenderTarget2D->InitAutoFormat(256, 256);

	SceneCaptureComponent2D->TextureTarget = PickingTextureRenderTarget2D;
}

void APickingActor::SetCapturedComponents() {
	TArray<UObject*> UObjectList;
	GetObjectsOfClass(UProceduralMeshComponent::StaticClass(), UObjectList, false, EObjectFlags::RF_ClassDefaultObject, EInternalObjectFlags::AllFlags);

	TArray<TWeakObjectPtr<UPrimitiveComponent> > ComponentList;
	for (UObject* Object : UObjectList)
	{
		UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(Object);
		if (!ComponentList.Contains(Component))
		{
			ComponentList.Add(Component);
		}
	}

	SceneCaptureComponent2D->ShowOnlyComponents = ComponentList;
	SceneCaptureComponent2D->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
}


/// ReadRenderTargetHelper

EPixelFormat ReadRenderTargetHelper(
	TArray<FLinearColor>& OutHDRValues,
	UObject* WorldContextObject,
	UTextureRenderTarget2D* TextureRenderTarget,
	int32 X,
	int32 Y,
	int32 Width,
	int32 Height,
	bool bNormalize = true)
{
	EPixelFormat OutFormat = PF_Unknown;

	if (!TextureRenderTarget)
	{
		return OutFormat;
	}

	FTextureRenderTarget2DResource* RTResource = (FTextureRenderTarget2DResource*)TextureRenderTarget->GameThread_GetRenderTargetResource();
	if (!RTResource)
	{
		return OutFormat;
	}

	X = FMath::Clamp(X, 0, TextureRenderTarget->SizeX - 1);
	Y = FMath::Clamp(Y, 0, TextureRenderTarget->SizeY - 1);
	Width = FMath::Clamp(Width, 1, TextureRenderTarget->SizeX);
	Height = FMath::Clamp(Height, 1, TextureRenderTarget->SizeY);
	Width = Width - FMath::Max(X + Width - TextureRenderTarget->SizeX, 0);
	Height = Height - FMath::Max(Y + Height - TextureRenderTarget->SizeY, 0);

	FIntRect SampleRect(X, Y, X + Width, Y + Height);

	FReadSurfaceDataFlags ReadSurfaceDataFlags = bNormalize ? FReadSurfaceDataFlags() : FReadSurfaceDataFlags(RCM_MinMax);

	FRenderTarget* RenderTarget = TextureRenderTarget->GameThread_GetRenderTargetResource();
	OutFormat = TextureRenderTarget->GetFormat();

	const int32 NumPixelsToRead = Width * Height;

	if (!RenderTarget->ReadLinearColorPixels(OutHDRValues, ReadSurfaceDataFlags, SampleRect))
	{
		OutFormat = PF_Unknown;
	}
	else
	{
		check(OutHDRValues.Num() == NumPixelsToRead);
	}

	return OutFormat;
}
