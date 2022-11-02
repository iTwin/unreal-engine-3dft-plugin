#pragma once

#include "VectorView.h"
#include <array>

struct MeshTile
{
public:
	using Vec2d = std::array<float, 2>;
	using Vec3d = std::array<float, 3>;

	struct Vertex
	{
		Vec3d Position;
		Vec3d Normal;
		uint32_t Color;
		uint64_t ElementId;
		float Error;
	};

	struct Mesh
	{
		VectorView<Vertex> Vertices;
		VectorView<uint32_t> Indexes;
		std::array<Vec3d, 2> BoundingBox;
	};

public:
	int32_t Id;
	int TriangleCount;

	struct
	{
		Vec3d Center;
		float Radius;
	} BatchPosition;

	struct
	{
		bool bTranslucent;
		int16_t Index;
	} Material;

	VectorView<Mesh> Meshes;
	VectorView<int32_t> BatchesToRemove;
	VectorView<int32_t> BatchesToShow;
};
