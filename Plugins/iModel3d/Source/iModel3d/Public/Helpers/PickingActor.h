/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <map>

#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "ProceduralMeshComponent.h"
#include "iModel3d.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickingActor.generated.h"

UCLASS()
class IMODEL3D_API APickingActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APickingActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
		void PickObjectAtMousePosition(FString& Id, FVector2D& MousePosition);

	UPROPERTY(EditInstanceOnly)
		AiModel3d* iModel3d;

	UPROPERTY(EditInstanceOnly)
		UTextureRenderTarget2D* PickingTextureRenderTarget2D;

	UPROPERTY(EditInstanceOnly)
		UMaterialParameterCollection* PickingMaterialParameterCollection;

	DECLARE_EVENT_OneParam(APickingActor, FElementPicked, FString);
	FElementPicked& OnElementPicked() { return ElementPickedEvent; }

private:
	USceneCaptureComponent2D* SceneCaptureComponent2D;
	FElementPicked ElementPickedEvent;

	void InitSceneCaptureComponent();
	void InitRenderTarget();
	void SetCapturedComponents();
	void CaptureScene();
	FVector2D PickObjectAt(const FVector2D& ScreenCoords);
};
