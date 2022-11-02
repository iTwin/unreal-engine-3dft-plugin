#pragma once

#include <cmath>

#include "CoreMinimal.h"


struct IModelElementIndex
{
	IModelElementIndex(uint32_t element_id)
	{
		_element_index = element_id;
	}

	IModelElementIndex(float X, float Y)
	{
		_element_index = uint32_t(std::round(X * float(Scale))) + uint32_t(std::round(Y * float(Scale))) * Scale - Reserved_count;
	}

	uint32_t id() const
	{
		return _element_index;
	}

	operator uint32_t() const
	{
		return _element_index;
	}

	operator FVector2f() const
	{
		return { ((_element_index + Reserved_count) % Scale) / float(Scale), (((_element_index + Reserved_count) / Scale) % Scale) / float(Scale) };
	}

	uint32_t scale() const
	{
		return Scale;
	}

private:
	// Number of IDs Reserved_count, starting from id=0.
	static constexpr uint32_t Reserved_count = 1;
	static constexpr uint32_t Scale = 2048;
	uint32_t _element_index;
};
