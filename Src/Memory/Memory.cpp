#include "Memory/Memory.h"
#include "Build.h"

#include <bit>

#include <mimalloc.h>

namespace Memory
{
	static size_t CountedSize(size_t count, size_t size, size_t alignment)
	{
		size_t alignedSize = (size + alignment - 1) / alignment * alignment;
		return count * alignedSize;
	}

	void* AlignedMalloc(std::size_t alignment, std::size_t size) noexcept
	{
		return mi_malloc_aligned(size, alignment);
	}

	void* AlignedZalloc(std::size_t alignment, std::size_t size) noexcept
	{
		return mi_zalloc_aligned(size, alignment);
	}

	void* AlignedCalloc(std::size_t alignment, std::size_t count, std::size_t size) noexcept
	{
		return mi_calloc_aligned(count, size, alignment);
	}

	void* AlignedZCalloc(std::size_t alignment, std::size_t count, std::size_t size) noexcept
	{
		return mi_calloc_aligned(count, size, alignment);
	}

	void* AlignedRMalloc(void* ptr, std::size_t newSize, std::size_t alignment) noexcept
	{
		return mi_realloc_aligned(ptr, newSize, alignment);
	}

	void* AlignedRZalloc(void* ptr, std::size_t newSize, std::size_t alignment) noexcept
	{
		return mi_rezalloc_aligned(ptr, newSize, alignment);
	}

	void* AlignedRCalloc(void* ptr, std::size_t newCount, std::size_t newSize, std::size_t alignment) noexcept
	{
		return mi_recalloc_aligned(ptr, newCount, newSize, alignment);
	}

	void* AlignedRZCalloc(void* ptr, std::size_t newCount, std::size_t newSize, std::size_t alignment) noexcept
	{
		return mi_recalloc_aligned(ptr, newCount, newSize, alignment);
	}

	void* AlignedEMalloc(void* ptr, std::size_t newSize, [[maybe_unused]] std::size_t alignment) noexcept
	{
		return mi_expand(ptr, newSize);
	}

	void* AlignedEZalloc(void* ptr, std::size_t newSize, [[maybe_unused]] std::size_t alignment) noexcept
	{
		return mi_expand(ptr, newSize);
	}

	void* AlignedECalloc(void* ptr, std::size_t newCount, std::size_t newSize, std::size_t alignment) noexcept
	{
		return mi_expand(ptr, CountedSize(newCount, newSize, alignment));
	}

	void* AlignedEZCalloc(void* ptr, std::size_t newCount, std::size_t newSize, std::size_t alignment) noexcept
	{
		return mi_expand(ptr, CountedSize(newCount, newSize, alignment));
	}

	void AlignedFree(void* ptr, std::size_t alignment) noexcept
	{
		mi_free_aligned(ptr, alignment);
	}

	void* Malloc(std::size_t size) noexcept
	{
		return AlignedMalloc(16, size);
	}

	void* Zalloc(std::size_t size) noexcept
	{
		return AlignedZalloc(16, size);
	}

	void* Calloc(std::size_t count, std::size_t size) noexcept
	{
		return AlignedCalloc(16, count, size);
	}

	void* ZCalloc(std::size_t count, std::size_t size) noexcept
	{
		return AlignedZCalloc(16, count, size);
	}

	void* RMalloc(void* ptr, std::size_t newSize) noexcept
	{
		return AlignedRMalloc(ptr, newSize, 16);
	}

	void* RZalloc(void* ptr, std::size_t newSize) noexcept
	{
		return AlignedRZalloc(ptr, newSize, 16);
	}

	void* RCalloc(void* ptr, std::size_t newCount, std::size_t newSize) noexcept
	{
		return AlignedRCalloc(ptr, newCount, newSize, 16);
	}

	void* RZCalloc(void* ptr, std::size_t newCount, std::size_t newSize) noexcept
	{
		return AlignedRZCalloc(ptr, newCount, newSize, 16);
	}

	void* EMalloc(void* ptr, std::size_t newSize) noexcept
	{
		return AlignedEMalloc(ptr, newSize, 16);
	}

	void* EZalloc(void* ptr, std::size_t newSize) noexcept
	{
		return AlignedEZalloc(ptr, newSize, 16);
	}

	void* ECalloc(void* ptr, std::size_t newCount, std::size_t newSize) noexcept
	{
		return AlignedECalloc(ptr, newCount, newSize, 16);
	}

	void* EZCalloc(void* ptr, std::size_t newCount, std::size_t newSize) noexcept
	{
		return AlignedEZCalloc(ptr, newCount, newSize, 16);
	}

	void Free(void* ptr) noexcept
	{
		return AlignedFree(ptr, 16);
	}
} // namespace Memory

extern "C"
{
	void* CBAlignedMalloc(size_t alignment, size_t size)
	{
		return Memory::AlignedMalloc(alignment, size);
	}

	void* AlignedZalloc(size_t alignment, size_t size)
	{
		return Memory::AlignedZalloc(alignment, size);
	}

	void* AlignedCalloc(size_t alignment, size_t count, size_t size)
	{
		return Memory::AlignedCalloc(alignment, count, size);
	}

	void* AlignedZCalloc(size_t alignment, size_t count, size_t size)
	{
		return Memory::AlignedZCalloc(alignment, count, size);
	}

	void* AlignedRMalloc(void* ptr, size_t newSize, size_t alignment)
	{
		return Memory::AlignedRMalloc(ptr, newSize, alignment);
	}

	void* AlignedRZalloc(void* ptr, size_t newSize, size_t alignment)
	{
		return Memory::AlignedRZalloc(ptr, newSize, alignment);
	}

	void* AlignedRCalloc(void* ptr, size_t newCount, size_t newSize, size_t alignment)
	{
		return Memory::AlignedRCalloc(ptr, newCount, newSize, alignment);
	}

	void* AlignedRZCalloc(void* ptr, size_t newCount, size_t newSize, size_t alignment)
	{
		return Memory::AlignedRZCalloc(ptr, newCount, newSize, alignment);
	}

	void* AlignedEMalloc(void* ptr, size_t newSize, size_t alignment)
	{
		return Memory::AlignedEMalloc(ptr, newSize, alignment);
	}

	void* AlignedEZalloc(void* ptr, size_t newSize, size_t alignment)
	{
		return Memory::AlignedEZalloc(ptr, newSize, alignment);
	}

	void* AlignedECalloc(void* ptr, size_t newCount, size_t newSize, size_t alignment)
	{
		return Memory::AlignedECalloc(ptr, newCount, newSize, alignment);
	}

	void* AlignedEZCalloc(void* ptr, size_t newCount, size_t newSize, size_t alignment)
	{
		return Memory::AlignedEZCalloc(ptr, newCount, newSize, alignment);
	}

	void AlignedFree(void* ptr, size_t alignment)
	{
		return Memory::AlignedFree(ptr, alignment);
	}

	void* Malloc(size_t size)
	{
		return Memory::Malloc(size);
	}

	void* Zalloc(size_t size)
	{
		return Memory::Zalloc(size);
	}

	void* Calloc(size_t count, size_t size)
	{
		return Memory::Calloc(count, size);
	}

	void* ZCalloc(size_t count, size_t size)
	{
		return Memory::ZCalloc(count, size);
	}

	void* RMalloc(void* ptr, size_t newSize)
	{
		return Memory::RMalloc(ptr, newSize);
	}

	void* RZalloc(void* ptr, size_t newSize)
	{
		return Memory::RZalloc(ptr, newSize);
	}

	void* RCalloc(void* ptr, size_t newCount, size_t newSize)
	{
		return Memory::RCalloc(ptr, newCount, newSize);
	}

	void* RZCalloc(void* ptr, size_t newCount, size_t newSize)
	{
		return Memory::RZCalloc(ptr, newCount, newSize);
	}

	void* EMalloc(void* ptr, size_t newSize)
	{
		return Memory::EMalloc(ptr, newSize);
	}

	void* EZalloc(void* ptr, size_t newSize)
	{
		return Memory::EZalloc(ptr, newSize);
	}

	void* ECalloc(void* ptr, size_t newCount, size_t newSize)
	{
		return Memory::ECalloc(ptr, newCount, newSize);
	}

	void* EZCalloc(void* ptr, size_t newCount, size_t newSize)
	{
		return Memory::EZCalloc(ptr, newCount, newSize);
	}

	void Free(void* ptr)
	{
		return Memory::Free(ptr);
	}
}
