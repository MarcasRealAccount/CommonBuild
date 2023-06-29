#pragma once

#include "Memory.h"

#include <cstddef>
#include <cstdint>

#include <limits>
#include <new>

namespace Memory
{
	template <class T>
	struct Allocator
	{
	public:
		using pointer            = T*;
		using const_pointer      = const T*;
		using void_pointer       = void*;
		using const_void_pointer = const void*;
		using value_type         = T;
		using size_type          = std::size_t;
		using difference_type    = std::ptrdiff_t;

		using is_always_equal = std::true_type;

		template <class U>
		struct rebind
		{
			using other = Allocator<U>;
		};

	public:
		Allocator() noexcept {}

		Allocator([[maybe_unused]] const Allocator& copy) noexcept {}

		Allocator([[maybe_unused]] Allocator&& move) noexcept {}

		template <class U>
		Allocator([[maybe_unused]] const Allocator<U>& copy) noexcept
		{
		}

		template <class U>
		Allocator([[maybe_unused]] const Allocator<U>&& move) noexcept
		{
		}

		Allocator& operator=([[maybe_unused]] const Allocator& copy) noexcept { return *this; }

		Allocator& operator=([[maybe_unused]] Allocator&& move) noexcept { return *this; }

		[[nodiscard]] pointer allocate(size_type n)
		{
			if (n > std::numeric_limits<size_type>::max() / sizeof(T))
				throw std::bad_array_new_length {};

			pointer p = reinterpret_cast<T*>(AlignedCalloc(alignof(T), n, sizeof(T)));
			if (!p)
				throw std::bad_alloc {};
			return p;
		}

		void deallocate(pointer p, [[maybe_unused]] size_type n) noexcept
		{
			AlignedFree(p, alignof(T));
		}

		size_type max_size() const noexcept
		{
			return std::numeric_limits<size_type>::max() / sizeof(T);
		}
	};

	template <class T, class U>
	bool operator==(const Allocator<T>& lhs, const Allocator<U>& rhs) noexcept
	{
		return true;
	}

	template <class T, class U>
	bool operator!=(const Allocator<T>& lhs, const Allocator<U>& rhs) noexcept
	{
		return !(lhs == rhs);
	}
} // namespace Memory