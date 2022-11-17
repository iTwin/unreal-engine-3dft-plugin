/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BatchLoaderRunnable.h"
#include "Helpers/IModelElementIndex.h"

namespace
{
	FVector Convert(std::array<float, 3> Vec)
	{
		return { Vec[0], Vec[1], Vec[2] };
	};

	struct MaterialProps
	{
		uint64_t ElementId;
		uint32_t SrcColor;
		uint32_t Color;
		float Specular;
		float Roughness;
		float Metalic;
		FVector Offset;
		float PixelOffset;
		bool bVisible;
	};

	MaterialProps GetMaterialOptions(uint32_t Color, uint64 ElementId, const FGraphicOptions& Options)
	{
		uint32_t OutColor = ((Color & 0xff000000) == 0) ? (Color | 0xff000000) : Color;
		FVector Offset = { 0, 0, 0 };
		float PixelOffset = 0;
		bool bVisible = true;
		if (Options.ElementInfos.Num() && Options.ElementInfos.Contains(ElementId))
		{
			const auto& Element = Options.ElementInfos[ElementId];
			Offset = Element.Offset;
			PixelOffset = Element.PixelOffset;
			bVisible = Element.bVisible;
			/*if (Element.bSetMaterial)
			{
				return { ElementId, Color, Element.Color.DWColor(), Element.Specular, Element.Roughness, Element.Metalic, Offset };
			}*/
		}
		if (Options.Materials.OverrideMaterials && !Options.Materials.DebugRGB)
		{
			for (const auto& Mat : Options.Materials.Overrides)
			{
				if (Mat.Enabled && Mat.Color == OutColor)
				{
					return { ElementId, Color, uint32_t(Mat.OutColor), Mat.Specular, Mat.Roughness, Mat.Metalic, Offset, PixelOffset, bVisible };
				}
			}

			const auto& Mat = Options.Materials.DefaultMaterial;
			if (Mat.Enabled)
			{
				return { ElementId, Color, uint32_t(Mat.OutColor), Mat.Specular, Mat.Roughness, Mat.Metalic, Offset, PixelOffset, bVisible };
			}
		}
		return { ElementId, Color, OutColor, 0, 1, 0, Offset, PixelOffset, bVisible };
	}

	FProceduralTileMesh MeshTileToCustomMeshTile(const MeshTile* Tile, std::unordered_map<uint64_t, uint32_t>& ElementIds, std::vector<uint64_t>& InvElementIds, FGraphicOptions Options)
	{
		FProceduralTileMesh PMesh =
		{
			Tile->Id,
			0,
			{ Tile->Material.bTranslucent, Tile->Material.Index},
			{
				{ Tile->BatchPosition.Center[0], Tile->BatchPosition.Center[1], Tile->BatchPosition.Center[2]},
				Tile->BatchPosition.Radius
			},
			{},
			TArray<int32>(Tile->BatchesToRemove.begin(), Tile->BatchesToRemove.size()),
			TArray<int32>(Tile->BatchesToShow.begin(), Tile->BatchesToShow.size())
		};

		PMesh.ProcMeshes.Reserve(Tile->Meshes.size());
		for (int i = 0; i < Tile->Meshes.size(); i++)
		{
			auto& m = Tile->Meshes[i];

			MaterialProps Material = {};
			struct
			{
				uint64_t Id;
				FVector2f Index;
			} Element = { 0, { 0.f, 0.f } };

			FCustomProcMeshSection Section;

			Section.Vertices.SetNumZeroed(m.Vertices.size());

			for (int32 j = 0; j < m.Vertices.size(); j++)
			{
				auto& Out = Section.Vertices[j];

				auto& Vertex = m.Vertices[j];
				if (j == 0 || Element.Id != Vertex.ElementId)
				{
					if (ElementIds.find(Vertex.ElementId) == ElementIds.end())
					{
						auto Index = uint32_t(InvElementIds.size());
						InvElementIds.push_back(Vertex.ElementId);
						ElementIds.emplace(Vertex.ElementId, Index);
					}

					Element = { Vertex.ElementId, IModelElementIndex(ElementIds.at(Vertex.ElementId)) };
				}

				float TimelineBatchId = 0.f;

				if (j == 0 || Material.SrcColor != Vertex.Color || Material.ElementId != Vertex.ElementId)
				{
					Material = GetMaterialOptions(Vertex.Color, Vertex.ElementId, Options);
				}


				Out.Position = (FVector3f&)Vertex.Position + FVector3f(Material.Offset.X, Material.Offset.Y, Material.Offset.Z);
				// Tangents and UV will be re-activated in the next version.
				// Out.TangentX = (FVector3f&)Vertex.Tangent.xyz;
				Out.TangentZ = (FVector3f&)Vertex.Normal;
				// Out.TangentY = (FVector3f)((Out.TangentX ^ Out.TangentZ) * (Vertex.Tangent.w ? -1 : 1));

				Out.UVs[0] = { Material.Specular, Material.Roughness };
				Out.UVs[1] = { Material.Metalic, Vertex.Error * 100.f + Material.PixelOffset };
				Out.UVs[2] = { TimelineBatchId, 0 };
				Out.UVs[3] = Element.Index;
				Out.Color = Material.bVisible ? FColor(Material.Color) : FColor(0);
			}

			Section.Indices = TArray<uint32>(m.Indexes.begin(), m.Indexes.size());
			Section.SectionLocalBox = FBox(Convert(m.BoundingBox[0]), Convert(m.BoundingBox[1]));
			Section.bSectionVisible = true;

			PMesh.TriangleNum += m.Indexes.size() / 3;

			PMesh.ProcMeshes.Push(MakeShared<FCustomProcMeshSection>(std::move(Section)));
		}
		return PMesh;
	}
}

FBatchLoaderRunnable::FBatchLoaderRunnable(const std::shared_ptr<IModelDecoder>& InModelDecoder)
	: ModelDecoder(InModelDecoder)
{
}

void FBatchLoaderRunnable::SetGraphicOptions(const TSharedPtr<FGraphicOptions>& GraphicOptions)
{
	OptionsLock.Lock();
	UpdatedOptions = *GraphicOptions;
	bUpdatedOptions = true;
	OptionsLock.Unlock();
}

uint64_t FBatchLoaderRunnable::GetElementId(uint32_t ElementIndex)
{
	ElementIdsLock.Lock();
	auto Id = ElementIndex < InvElementIds.size() ? InvElementIds.at(ElementIndex) : 0;
	ElementIdsLock.Unlock();
	return Id;
}

uint32 FBatchLoaderRunnable::Run()
{
	while (bIsRunning)
	{
		while (true)
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_FMeshComponentManager_Produce);

			if (ReadyTrianglesFull() || !ModelDecoder->IsNextBatchReady())
			{
				break;
			}

			if (bUpdatedOptions)
			{
				OptionsLock.Lock();
				Options = std::move(UpdatedOptions);
				bUpdatedOptions = false;
				OptionsLock.Unlock();
			}

			auto MeshTile = ModelDecoder->GetNextRenderableBatch();

			ElementIdsLock.Lock();
			auto NewBatch = MakeShared<FProceduralTileMesh>(MeshTileToCustomMeshTile(MeshTile, ElementIds, InvElementIds, Options));
			ElementIdsLock.Unlock();
			ModelDecoder->Release(MeshTile);

			ReadyRenderableBatchesLock.Lock();
			ReadyTriangles += NewBatch->TriangleNum;
			ReadyRenderableBatches.emplace_back(std::move(NewBatch));
			ReadyRenderableBatchesLock.Unlock();
		}

		FPlatformProcess::SleepNoStats(0.004f); // 250 fps
	}

	return 0;
}

void FBatchLoaderRunnable::Stop()
{
	bIsRunning = false;
}

TSharedPtr<FProceduralTileMesh> FBatchLoaderRunnable::PopMesh()
{
	ReadyRenderableBatchesLock.Lock();
	auto NewBatch = ReadyRenderableBatches.front();
	ReadyRenderableBatches.pop_front();
	for (int i = 0; i < NewBatch->ProcMeshes.Num(); i++)
	{
		ReadyTriangles -= NewBatch->ProcMeshes[i]->Indices.Num() / 3;
	}
	ReadyRenderableBatchesLock.Unlock();
	return NewBatch;
}

size_t FBatchLoaderRunnable::NumReadyMeshes()
{
	ReadyRenderableBatchesLock.Lock();
	auto Num = ReadyRenderableBatches.size();
	ReadyRenderableBatchesLock.Unlock();

	return Num;
}

bool FBatchLoaderRunnable::ReadyTrianglesFull()
{
	ReadyRenderableBatchesLock.Lock();
	auto bEnough = ReadyTriangles >= TriangleBudget;
	ReadyRenderableBatchesLock.Unlock();

	return bEnough;
}

void FBatchLoaderRunnable::SetTriangleBudget(size_t Budget)
{
	ReadyRenderableBatchesLock.Lock();
	TriangleBudget = Budget;
	ReadyRenderableBatchesLock.Unlock();
}
