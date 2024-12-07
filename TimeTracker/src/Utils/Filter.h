#pragma once
#include <functional>

template <typename... T>
class Filter
{
public:
	using FilterFunction = std::function<bool(T...)>;

	static Filter empty()
	{
		return Filter([](T...) { return true; });
	}

	static Filter create(FilterFunction filter)
	{
		return Filter(filter);
	}

	Filter(FilterFunction filter)
		: _filter(filter)
	{
	}

	bool operator()(T... args) const
	{
		return _filter(args...);
	}

private:
	FilterFunction _filter;
};