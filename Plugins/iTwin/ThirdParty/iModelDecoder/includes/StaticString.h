/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <array>
#include <string>
#include <cstring>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

class StaticString : public std::array<char, 256>
{
public:
	inline const char* c_str() const
	{
		return &(*this)[0];
	};

	inline static StaticString create(const char* s)
	{
		StaticString str;
		for (int i = 0; i < 256; i++)
		{
			str[i] = s[i];
			if (!s[i])
			{
				break;
			}
		}
		return str;
	};

	inline static StaticString create(const std::string& s)
	{
		return StaticString::create(s.c_str());
	};

	inline auto length()
	{
		return strlen(c_str());
	};
};
