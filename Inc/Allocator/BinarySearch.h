#pragma once

#include <cstddef>

namespace Allocator
{
	// Binary search for first element >= key
	template <class K, class Comp>
	std::size_t BSGreaterEqual(const K& key, std::size_t first, std::size_t last, Comp comp) noexcept
	{
		while (first != last)
		{
			std::size_t mid = first + ((last - first) >> 1);
			if (comp(mid, key) < 0)
				first = mid + 1;
			else
				last = mid;
		}
		return first;
	}

	// Binary search for first element > key
	template <class K, class Comp>
	std::size_t BSGreater(const K& key, std::size_t first, std::size_t last, Comp comp) noexcept
	{
		while (first != last)
		{
			std::size_t mid = first + ((last - first) >> 1);
			if (comp(mid, key) <= 0)
				first = mid + 1;
			else
				last = mid;
		}
		return first;
	}

	// Binary search for last element < or first element == key
	template <class K, class Comp>
	std::size_t BSLessEqual(const K& key, std::size_t first, std::size_t last, Comp comp) noexcept
	{
		if (first == last)
			return first;

		--last;
		while (first != last)
		{
			std::size_t mid = first + ((last - first + 1) >> 1);
			if (comp(mid, key) > 0)
				last = mid - 1;
			else
				first = mid;
		}
		return first;
	}

	// Binary search for first element == key
	template <class K, class Comp>
	std::size_t BSEqual(const K& key, std::size_t first, std::size_t last, Comp comp) noexcept
	{
		std::size_t res = BSGreaterEqual<K, Comp>(key, first, last, comp);
		return comp(res, key) == 0 ? res : last;
	}
} // namespace Allocator