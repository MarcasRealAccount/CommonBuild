#pragma once

#include <cstddef>

namespace Memory
{
	void* AlignedMalloc(std::size_t alignment, std::size_t size);
	void* AlignedZalloc(std::size_t alignment, std::size_t size);
	void* AlignedCalloc(std::size_t alignment, std::size_t count, std::size_t size);
	void* AlignedZCalloc(std::size_t alignment, std::size_t count, std::size_t size);

	void* AlignedRMalloc(void* ptr, std::size_t alignment, std::size_t newSize);
	void* AlignedRZalloc(void* ptr, std::size_t alignment, std::size_t newSize);
	void* AlignedRCalloc(void* ptr, std::size_t alignment, std::size_t newCount, std::size_t newSize);
	void* AlignedRZCalloc(void* ptr, std::size_t alignment, std::size_t newCount, std::size_t newSize);

	void* AlignedEMalloc(void* ptr, std::size_t alignment, std::size_t newSize);
	void* AlignedEZalloc(void* ptr, std::size_t alignment, std::size_t newSize);
	void* AlignedECalloc(void* ptr, std::size_t alignment, std::size_t newCount, std::size_t newSize);
	void* AlignedEZCalloc(void* ptr, std::size_t alignment, std::size_t newCount, std::size_t newSize);

	void AlignedFree(void* ptr, std::size_t alignment);

	void* Malloc(std::size_t size);
	void* Zalloc(std::size_t size);
	void* Calloc(std::size_t count, std::size_t size);
	void* ZCalloc(std::size_t count, std::size_t size);

	void* RMalloc(void* ptr, std::size_t newSize);
	void* RZalloc(void* ptr, std::size_t newSize);
	void* RCalloc(void* ptr, std::size_t newCount, std::size_t newSize);
	void* RZCalloc(void* ptr, std::size_t newCount, std::size_t newSize);

	void* EMalloc(void* ptr, std::size_t newSize);
	void* EZalloc(void* ptr, std::size_t newSize);
	void* ECalloc(void* ptr, std::size_t newCount, std::size_t newSize);
	void* EZCalloc(void* ptr, std::size_t newCount, std::size_t newSize);

	void Free(void* ptr);
} // namespace Memory