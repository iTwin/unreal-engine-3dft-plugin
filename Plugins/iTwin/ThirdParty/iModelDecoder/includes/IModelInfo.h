/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <array>

struct IModelInfo
{
public:
	using Vec3d = std::array<double, 3>;

public:
	std::array<Vec3d, 2> BoundingBox;
	Vec3d ModelCenter;
};
