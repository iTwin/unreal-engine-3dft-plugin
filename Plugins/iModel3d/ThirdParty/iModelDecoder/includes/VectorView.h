#pragma once

#include <vector>

template <typename T>
class VectorView
{
public:
	inline VectorView(const std::vector<T>& v)
		: data(v.data())
		, count(static_cast<uint32_t>(v.size())){};
	inline VectorView()
		: data(nullptr)
		, count(0){};

	inline operator std::vector<T>() const
	{
		return std::vector<T>(data, data + count);
	};
	inline size_t size() const
	{
		return count;
	};
	inline const T& operator[](int n) const
	{
		return data[n];
	};
	inline const T* begin() const
	{
		return data;
	};
	inline const T* end() const
	{
		return data + count;
	};

private:
	const T* data;
	uint32_t count;
};
