#pragma once

#ifdef __cplusplus
	#include <cstddef>

	#include <concepts>

namespace Memory
{
	namespace Details
	{
		template <class T>
		concept Numeric = std::integral<T> || std::floating_point<T>;
	}

	template <Details::Numeric T1, Details::Numeric T2>
	constexpr auto AlignCeil(T1 value, T2 alignment) noexcept
	{
		auto mask = alignment - 1;
		auto val  = value + mask;
		if ((alignment & mask) == 0)
			return val & ~mask;
		else
			return val / alignment * alignment;
	}

	template <Details::Numeric T1, Details::Numeric T2>
	constexpr auto AlignFloor(T1 value, T2 alignment) noexcept
	{
		auto mask = alignment - 1;
		if ((alignment & mask) == 0)
			return value & ~mask;
		else
			return value / alignment * alignment;
	}

	template <Details::Numeric T1, Details::Numeric T2>
	constexpr auto AlignCountCeil(T1 value, T2 alignment) noexcept
	{
		auto mask = alignment - 1;
		auto val  = value + mask;
		if ((alignment & mask) == 0)
			return val >> alignment;
		else
			return val / alignment;
	}

	template <Details::Numeric T1, Details::Numeric T2>
	constexpr auto AlignCountFloor(T1 value, T2 alignment) noexcept
	{
		auto mask = alignment - 1;
		if ((alignment & mask) == 0)
			return value >> alignment;
		else
			return value / alignment;
	}

	[[nodiscard]] void* AlignedMalloc(std::size_t alignment, std::size_t size) noexcept;
	[[nodiscard]] void* AlignedZalloc(std::size_t alignment, std::size_t size) noexcept;
	[[nodiscard]] void* AlignedCalloc(std::size_t alignment, std::size_t count, std::size_t size) noexcept;
	[[nodiscard]] void* AlignedZCalloc(std::size_t alignment, std::size_t count, std::size_t size) noexcept;

	[[nodiscard]] void* AlignedRMalloc(void* ptr, std::size_t newSize, std::size_t alignment = 0) noexcept;
	[[nodiscard]] void* AlignedRZalloc(void* ptr, std::size_t newSize, std::size_t alignment = 0) noexcept;
	[[nodiscard]] void* AlignedRCalloc(void* ptr, std::size_t newCount, std::size_t newSize, std::size_t alignment = 0) noexcept;
	[[nodiscard]] void* AlignedRZCalloc(void* ptr, std::size_t newCount, std::size_t newSize, std::size_t alignment = 0) noexcept;

	[[nodiscard]] void* AlignedEMalloc(void* ptr, std::size_t newSize, std::size_t alignment = 0) noexcept;
	[[nodiscard]] void* AlignedEZalloc(void* ptr, std::size_t newSize, std::size_t alignment = 0) noexcept;
	[[nodiscard]] void* AlignedECalloc(void* ptr, std::size_t newCount, std::size_t newSize, std::size_t alignment = 0) noexcept;
	[[nodiscard]] void* AlignedEZCalloc(void* ptr, std::size_t newCount, std::size_t newSize, std::size_t alignment = 0) noexcept;

	void AlignedFree(void* ptr, std::size_t alignment = 0) noexcept;

	[[nodiscard]] void* Malloc(std::size_t size) noexcept;
	[[nodiscard]] void* Zalloc(std::size_t size) noexcept;
	[[nodiscard]] void* Calloc(std::size_t count, std::size_t size) noexcept;
	[[nodiscard]] void* ZCalloc(std::size_t count, std::size_t size) noexcept;

	[[nodiscard]] void* RMalloc(void* ptr, std::size_t newSize) noexcept;
	[[nodiscard]] void* RZalloc(void* ptr, std::size_t newSize) noexcept;
	[[nodiscard]] void* RCalloc(void* ptr, std::size_t newCount, std::size_t newSize) noexcept;
	[[nodiscard]] void* RZCalloc(void* ptr, std::size_t newCount, std::size_t newSize) noexcept;

	[[nodiscard]] void* EMalloc(void* ptr, std::size_t newSize) noexcept;
	[[nodiscard]] void* EZalloc(void* ptr, std::size_t newSize) noexcept;
	[[nodiscard]] void* ECalloc(void* ptr, std::size_t newCount, std::size_t newSize) noexcept;
	[[nodiscard]] void* EZCalloc(void* ptr, std::size_t newCount, std::size_t newSize) noexcept;

	void Free(void* ptr) noexcept;
} // namespace Memory

#else

	#include <cstddef>

	#if __STDC_VERSION__ > 202311L
		#define NODISCARD [[nodiscard]]
	#else
		#define NODISCARD
	#endif

NODISCARD void* CBAlignedMalloc(size_t alignment, size_t size);
NODISCARD void* AlignedZalloc(size_t alignment, size_t size);
NODISCARD void* AlignedCalloc(size_t alignment, size_t count, size_t size);
NODISCARD void* AlignedZCalloc(size_t alignment, size_t count, size_t size);

NODISCARD void* AlignedRMalloc(void* ptr, size_t newSize, size_t alignment);
NODISCARD void* AlignedRZalloc(void* ptr, size_t newSize, size_t alignment);
NODISCARD void* AlignedRCalloc(void* ptr, size_t newCount, size_t newSize, size_t alignment);
NODISCARD void* AlignedRZCalloc(void* ptr, size_t newCount, size_t newSize, size_t alignment);

NODISCARD void* AlignedEMalloc(void* ptr, size_t newSize, size_t alignment);
NODISCARD void* AlignedEZalloc(void* ptr, size_t newSize, size_t alignment);
NODISCARD void* AlignedECalloc(void* ptr, size_t newCount, size_t newSize, size_t alignment);
NODISCARD void* AlignedEZCalloc(void* ptr, size_t newCount, size_t newSize, size_t alignment);

void AlignedFree(void* ptr, size_t alignment);

NODISCARD void* Malloc(size_t size);
NODISCARD void* Zalloc(size_t size);
NODISCARD void* Calloc(size_t count, size_t size);
NODISCARD void* ZCalloc(size_t count, size_t size);

NODISCARD void* RMalloc(void* ptr, size_t newSize);
NODISCARD void* RZalloc(void* ptr, size_t newSize);
NODISCARD void* RCalloc(void* ptr, size_t newCount, size_t newSize);
NODISCARD void* RZCalloc(void* ptr, size_t newCount, size_t newSize);

NODISCARD void* EMalloc(void* ptr, size_t newSize);
NODISCARD void* EZalloc(void* ptr, size_t newSize);
NODISCARD void* ECalloc(void* ptr, size_t newCount, size_t newSize);
NODISCARD void* EZCalloc(void* ptr, size_t newCount, size_t newSize);

void Free(void* ptr);

	#undef NODISCARD

#endif