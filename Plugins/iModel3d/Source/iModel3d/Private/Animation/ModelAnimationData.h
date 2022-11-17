/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <unordered_map>
#include <vector>


struct FVisibilityFrame
{
	int32_t Time;
	uint8_t Interpolation;
	uint8_t Value;
};

struct FColorFrame
{
	int32_t Time;
	uint8_t Interpolation;
	uint32_t Value;
};

struct FAnimationTimeline
{
	std::vector<uint64_t> ElementIds;
	std::vector<FVisibilityFrame> Visibility;
	std::vector<FColorFrame> Color;
};

struct FAnimatedData
{
	std::vector<uint8_t> Visibility;
	std::vector<uint32_t> Color;
};

struct FModelAnimationData
{
	std::vector<FAnimationTimeline> Timelines;
	std::unordered_map<uint64_t, uint32_t> ElementTimeline;
	bool bAnimationLoaded = false;

public:

	void ReadFile(const std::vector<uint8_t>& AssetFile);

	FAnimatedData GetAnimatedData(int32_t Time);

	bool GetStartEndTimes(int32_t& OutStart, int32_t& OutEnd);

	bool IsAnimationLoaded() const
	{
		return bAnimationLoaded;
	}
};
