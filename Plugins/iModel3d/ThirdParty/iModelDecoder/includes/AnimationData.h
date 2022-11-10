#pragma once

#include "VectorView.h"

class AnimationData
{
public:
	AnimationData(const std::vector<uint8_t>& visibility, const std::vector<uint32_t>& color)
		: Visibility(visibility)
		, Color(color){};

public:
	VectorView<uint8_t> Visibility;
	VectorView<uint32_t> Color;
};