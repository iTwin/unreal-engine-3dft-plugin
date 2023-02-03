/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ModelAnimationData.h"

namespace
{
	template <typename T>
	T Read(const uint8_t*& Data)
	{
		auto Value = *reinterpret_cast<const T*>(Data);
		Data += sizeof(T);
		return Value;
	}

	template <typename T, typename E>
	std::vector<T> ReadArray(const uint8_t*& Data)
	{
		auto Size = Read<uint32_t>(Data);
		auto Ptr = reinterpret_cast<const T*>(Data);
		auto Values = std::vector<T>(Ptr, Ptr + Size / (sizeof(T) / sizeof(E)));
		Data += Size * sizeof(E);
		return Values;
	}

	std::string ReadString(const uint8_t*& Data)
	{
		auto Chars = ReadArray<char, char>(Data);
		Chars.push_back(0);
		return Chars.data();
	}

	template <typename T>
	void GetArrayStartEndTimes(const std::vector<T>& Frames, int32_t& OutStart, int32_t& OutEnd, bool& bFirst)
	{
		if (Frames.size())
		{
			if (bFirst)
			{
				OutStart = OutEnd = Frames[0].Time;
				bFirst = false;
			}
			else
			{
				OutStart = std::min(OutStart, Frames[0].Time);
				OutEnd = std::max(OutEnd, Frames.back().Time);
			}
		}
	}

	template <typename T>
	uint32_t GetTimeValue(const std::vector<T>& Frames, int32_t Time, uint32_t DefaultValue)
	{
		uint32_t Value = DefaultValue;
		for (int j = 0; j < Frames.size(); j++)
		{
			const auto& Frame = Frames[j];
			if (Frame.Time > Time)
			{
				break;
			}
			else
			{
				Value = Frame.Value;
			}
		}
		return Value;
	}
} // namespace

FAnimatedData FModelAnimationData::GetAnimatedData(int32_t Time)
{
	std::vector<uint8_t> Visibility;
	std::vector<uint32_t> Color;

	Visibility.reserve(Timelines.size());
	Color.reserve(Timelines.size());

	for (int i = 0; i < Timelines.size(); i++)
	{
		Visibility.push_back(GetTimeValue(Timelines[i].Visibility, Time, 255));
		Color.push_back(GetTimeValue(Timelines[i].Color, Time, 0));
	}
	return { std::move(Visibility), std::move(Color) };
}

bool FModelAnimationData::GetStartEndTimes(int32_t& OutStart, int32_t& OutEnd)
{
	bool bFirst = true;
	OutStart = OutEnd = 0;
	for (int i = 0; i < Timelines.size(); i++)
	{
		GetArrayStartEndTimes(Timelines[i].Visibility, OutStart, OutEnd, bFirst);
		GetArrayStartEndTimes(Timelines[i].Color, OutStart, OutEnd, bFirst);
	}

	return !bFirst;
}

void FModelAnimationData::ReadFile(const std::vector<uint8_t>& AssetFile)
{
	const uint8_t* Data = AssetFile.data();
	auto Count = Read<uint32_t>(Data);
	Timelines.reserve(Count);

	for (uint32_t i = 0; i < Count; i++)
	{
		FAnimationTimeline timeline = { ReadArray<uint64_t, uint64_t>(Data) };

		auto VisibilityFrameCount = Read<uint32_t>(Data);
		for (uint32_t j = 0; j < VisibilityFrameCount; j++)
		{
			timeline.Visibility.push_back({ Read<int32_t>(Data), Read<uint8_t>(Data), Read<uint8_t>(Data) });
		}

		auto ColorFrameCount = Read<uint32_t>(Data);
		for (uint32_t j = 0; j < ColorFrameCount; j++)
		{
			timeline.Color.push_back({ Read<int32_t>(Data), Read<uint8_t>(Data), Read<uint32_t>(Data) });
		}
		Timelines.push_back(std::move(timeline));
	}

	for (int i = 0; i < Timelines.size(); i++)
	{
		const auto& timeline = Timelines[i];
		if (timeline.Visibility.size())
		{
			for (const auto& Id : timeline.ElementIds)
			{
				ElementTimeline.emplace(Id, static_cast<uint32_t>(i));
			}
		}
	}

	bAnimationLoaded = true;
}
