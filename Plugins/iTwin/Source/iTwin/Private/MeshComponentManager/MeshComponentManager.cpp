/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "MeshComponentManager.h"
#include "Common/Debug.h"
#include "CreateIModelDecoder.h"

#include <cmath>

DECLARE_STATS_GROUP(TEXT("MeshComponentManager"), STATGROUP_MeshComponentManager, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("Tick"), STAT_MeshComponentManager_Tick, STATGROUP_MeshComponentManager);

FMeshComponentManager::FMeshComponentManager(AActor* InActor, bool bIsInEditor)
	: Actor(InActor)
{
	RedirectStdOutput();

	ProceduralMeshes = std::make_shared<FProceduralMeshes>(InActor, bIsInEditor);

	ModelDecoder = CreateIModelDecoder();

	BatchLoaderRunnable = std::make_unique<FBatchLoaderRunnable>(ModelDecoder);
	BatchLoaderThread = FRunnableThread::Create(BatchLoaderRunnable.get(), TEXT("Batch Loader Runnable Thread"));

	DataRequests = MakeShared<FDataRequestManager>();
}

FMeshComponentManager::~FMeshComponentManager()
{
	BatchLoaderThread->Kill(true);
	ProceduralMeshes->Reset();
}

void FMeshComponentManager::SetMaterials(UMaterialInterface* Opaque, UMaterialInterface* Translucent)
{
	Materials.Opaque.Initialize(Opaque);
	Materials.Translucent.Initialize(Translucent);
}

void FMeshComponentManager::SetOptions(FIMOptions InOptions, FPluginOptions InPluginOptions)
{
	Options = InOptions;
	PluginOptions = InPluginOptions;
	ModelDecoder->SetOptions(Options);
	BatchLoaderRunnable->SetTriangleBudget(3 * LoadingVerticesPerFrame * (PluginOptions.ObjectLoadingSpeed ? PluginOptions.ObjectLoadingSpeed : 1));

	DataRequests->SetUseCache(PluginOptions.bUseDiskCache);
}

void FMeshComponentManager::OnLoaded(std::function<void(bool)> Callback)
{
	OnLoadedCallback = Callback;
}

void FMeshComponentManager::SetUrl(FString Url)
{
	DataRequests->SetBaseUrl(Url);
}

void FMeshComponentManager::SetGraphicOptions(const TSharedPtr<FGraphicOptions>& GraphicOptions)
{
	BatchLoaderRunnable->SetGraphicOptions(GraphicOptions);
}

void FMeshComponentManager::AddBinaryAssets(const uint8* Data, size_t Size, FString RelativePath)
{
	ModelDecoder->AddBinaryAssets(Data, Size, TCHAR_TO_UTF8(*RelativePath));
}

int FMeshComponentManager::GetNumInstances()
{
	return 0;
}

int FMeshComponentManager::GetNumDrawcalls()
{
	return VisibleMeshes.Num();
}

void FMeshComponentManager::Update(float DeltaTime, FVector CamLocation, FVector CamForward)
{
	SCOPE_CYCLE_COUNTER(STAT_MeshComponentManager_Tick);

	while (true)
	{
		auto Url = ModelDecoder->GetPendingRequest();
		if (Url.length() > 0)
		{
			DataRequests->AddRequest(Url.c_str(), [this](const uint8* Data, size_t Size, FString RelativePath) {
				AddBinaryAssets(Data, Size, RelativePath);
				if (RelativePath == "scene_description" && OnLoadedCallback)
				{
					FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this, Size](float Delta) -> bool { OnLoadedCallback(Size > 0); return false; }), 0);
				}
			});
		}
		else
		{
			break;
		}
	}
	ProceduralMeshes->Update();

	ModelDecoder->SetCameraLocation({ (float)CamLocation.X, (float)CamLocation.Y, (float)CamLocation.Z }, { (float)CamForward.X, (float)CamForward.Y, (float)CamForward.Z });

	UpdateBatches(DeltaTime, CamLocation);

	UpdateShadows(CamLocation);
}

/*
void FMeshComponentManager::HideVisible()
{
	for (auto& Component : VisibleMeshes)
	{
		ProceduralMeshes.Push(Component.MeshComponent);
	}
	VisibleMeshes.Empty();

	ProceduralMeshes.Update();
}
*/

void FMeshComponentManager::UpdateBatches(float DeltaTime, FVector CamLocation)
{
	constexpr auto FrameTimeTarget = 1.0 / FPSWhenLoadingMeshes;
	float CreationCount = DeltaTime > 0 ? FrameTimeTarget / DeltaTime : 1.0;

	while (CreationCount >= 0 && (BatchLoaderRunnable->NumReadyMeshes() > 0 || CurrentRenderableBatch) && ProceduralMeshes->PoolNum() > 0)
	{
		if (!CurrentRenderableBatch)
		{
			CurrentRenderableBatch = BatchLoaderRunnable->PopMesh();
		}

		if (CurrentRenderableBatch->ProcMeshes.Num())
		{
			auto ProcMesh = CurrentRenderableBatch->ProcMeshes.Pop();

			if (ProcMesh->Vertices.Num() && ProcMesh->Indices.Num())
			{
				auto TriangleCoeficient = (ProcMesh->Indices.Num() / 3) / float(LoadingVerticesPerFrame);
				auto Speed = PluginOptions.ObjectLoadingSpeed > 0 ? PluginOptions.ObjectLoadingSpeed : 1.0;
				CreationCount -= TriangleCoeficient / Speed;

				const auto& Material = CurrentRenderableBatch->Material;
				auto MeshComponent = ProceduralMeshes->Pop();

				auto& Pos = CurrentRenderableBatch->BatchPosition;

				FBatchPMComponent BatchComponent = { MeshComponent, Material, {{Pos.Center[0], Pos.Center[1], Pos.Center[2]}, Pos.Radius}, false, CurrentRenderableBatch->Id
				};

				UpdateComponentShadows(CamLocation, BatchComponent);

				MeshComponent->SetProcMeshSection(0, std::move(ProcMesh));
				MeshComponent->SetMeshSectionVisible(0, false);

				MeshComponent->SetMaterial(0, Material.bTranslucent ? Materials.Translucent.Get() : Materials.Opaque.Get());

				LoadingMeshes.Push(std::move(BatchComponent));
			}
		}

		if (!CurrentRenderableBatch->ProcMeshes.Num())
		{
			if (CurrentRenderableBatch->BatchesToRemove.Num())
			{
				for (int i = 0; i < VisibleMeshes.Num(); i++)
				{
					auto& Component = VisibleMeshes[i];
					auto& BatchesToRemove = CurrentRenderableBatch->BatchesToRemove;
					if (BatchesToRemove.Contains(Component.BatchId))
					{
						ProceduralMeshes->Push(Component.MeshComponent);
						VisibleMeshes.RemoveAt(i--);
					}
				}
			}

			if (CurrentRenderableBatch->BatchesToShow.Num())
			{
				for (int i = 0; i < LoadingMeshes.Num(); i++)
				{
					auto& Component = LoadingMeshes[i];

					auto& BatchesToShow = CurrentRenderableBatch->BatchesToShow;
					if (BatchesToShow.Contains(Component.BatchId))
					{
						ProceduralMeshes->Show(Component.MeshComponent);
						VisibleMeshes.Add(std::move(Component));
						LoadingMeshes.RemoveAt(i--);
					}
				}
			}
			CurrentRenderableBatch.Reset();
		}
	}
}

void FMeshComponentManager::UpdateShadows(FVector CamLocation)
{
	for (auto& Component : VisibleMeshes)
	{
		UpdateComponentShadows(CamLocation, Component);
	}
}

void FMeshComponentManager::UpdateComponentShadows(FVector CamLocation, FBatchPMComponent& Component)
{
	CamLocation.Z = 0;
	auto Distance = FVector::Distance(CamLocation, Component.Position.Center) - Component.Position.Radius;
	if (!Component.bShadowsOn && Distance < PluginOptions.ShadowDistanceCulling)
	{
		Component.bShadowsOn = true;
		Component.MeshComponent->bCastDynamicShadow = true;
		Component.MeshComponent->SetCastShadow(true);
	}
	else if (Component.bShadowsOn && Distance > PluginOptions.ShadowDistanceCulling * 1.25)
	{
		Component.bShadowsOn = false;
		Component.MeshComponent->bCastDynamicShadow = false;
		Component.MeshComponent->SetCastShadow(false);
	}
}

void FMeshComponentManager::GetAnimationInfo(int32& Start, int32& End, int32& Duration, bool& Loaded)
{
	Animation.GetAnimationInfo(Start, End, Duration, Loaded);
}

void FMeshComponentManager::GetStatusInfo(FString& Status, float& Percentage)
{
	auto status = ModelDecoder->GetStatusInfo(Percentage);
	Status = status.c_str();
}

namespace
{
	template<typename T>
	auto ToVector(const T& v)
	{
		return FVector(v[0], v[1], v[2]);
	}
}

 FiModel3DInfo FMeshComponentManager::GetModel3DInfo()
{
	auto Info = ModelDecoder->GetiModelInfo();

	return FiModel3DInfo{ ToVector(Info.BoundingBox[0]), ToVector(Info.BoundingBox[1]), ToVector(Info.ModelCenter) };
}

void FMeshComponentManager::SetPlaybackPosition(int32 Time)
{
	Animation.SetPlaybackPosition(Time);

	if (Animation.AreTexturesDirty())
	{
		Animation.ClearTexturesDirty();

		Materials.Translucent.Get()->SetTextureParameterValue("VisibilityTex", Animation.VisibilityTexture->Get());
		Materials.Opaque.Get()->SetTextureParameterValue("VisibilityTex", Animation.VisibilityTexture->Get());

		Materials.Translucent.Get()->SetTextureParameterValue("ColorTex", Animation.ColorTexture->Get());
		Materials.Opaque.Get()->SetTextureParameterValue("ColorTex", Animation.ColorTexture->Get());
	}
}

uint64_t FMeshComponentManager::GetElementId(uint32_t ElementIndex)
{
	return BatchLoaderRunnable->GetElementId(ElementIndex);
}
