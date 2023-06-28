#include "Memory/Memory.h"
#include "Build.h"

#if BUILD_IS_SYSTEM_WINDOWS
	#include <cstdlib>
	#include <cstring>
#endif

namespace Memory
{
	void* AlignedMalloc(std::size_t alignment, std::size_t size) noexcept
	{
		if constexpr (Common::c_IsSystemWindows)
			return _aligned_malloc(size, alignment);
		else
			return nullptr;
	}

	void* AlignedZalloc(std::size_t alignment, std::size_t size) noexcept
	{
		if constexpr (Common::c_IsSystemWindows)
		{
			void* ptr = AlignedMalloc(alignment, size);
			if (ptr)
				std::memset(ptr, 0, size);
			return ptr;
		}
		else
		{
			return nullptr;
		}
	}

	void* AlignedCalloc(std::size_t alignment, std::size_t count, std::size_t size) noexcept
	{
		if constexpr (Common::c_IsSystemWindows)
			return AlignedMalloc(alignment, count * AlignCeil(size, alignment));
		else
			return nullptr;
	}

	void* AlignedZCalloc(std::size_t alignment, std::size_t count, std::size_t size) noexcept
	{
		if constexpr (Common::c_IsSystemWindows)
		{
			std::size_t allocSize = count * AlignCeil(size, alignment);
			void*       ptr       = AlignedMalloc(alignment, allocSize);
			if (ptr)
				std::memset(ptr, 0, allocSize);
			return ptr;
		}
		else
		{
			return nullptr;
		}
	}

	void* AlignedRMalloc(void* ptr, std::size_t alignment, std::size_t newSize) noexcept
	{
		if constexpr (Common::c_IsSystemWindows)
			return _aligned_realloc(ptr, newSize, alignment);
		else
			return nullptr;
	}

	void* AlignedRZalloc(void* ptr, std::size_t alignment, std::size_t newSize) noexcept
	{
		if constexpr (Common::c_IsSystemWindows)
		{
			void* ptr = AlignedRMalloc(ptr, alignment, newSize);
			if (ptr)
				std::memset(ptr, 0, newSize);
			return ptr;
		}
		else
		{
			return nullptr;
		}
	}

	void* AlignedRCalloc(void* ptr, std::size_t alignment, std::size_t newCount, std::size_t newSize) noexcept
	{
		if constexpr (Common::c_IsSystemWindows)
			return AlignedRMalloc(ptr, alignment, newCount * AlignCeil(newSize, alignment));
		else
			return nullptr;
	}

	void* AlignedRZCalloc(void* ptr, std::size_t alignment, std::size_t newCount, std::size_t newSize) noexcept
	{
		if constexpr (Common::c_IsSystemWindows)
		{
			std::size_t allocSize = newCount * AlignCeil(newSize, alignment);
			void*       ptr       = AlignedRMalloc(ptr, alignment, allocSize);
			if (ptr)
				std::memset(ptr, 0, allocSize);
			return ptr;
		}
		else
		{
			return nullptr;
		}
	}

	void* AlignedEMalloc(void* ptr, std::size_t alignment, std::size_t newSize) noexcept
	{
		return nullptr;
	}

	void* AlignedEZalloc(void* ptr, std::size_t alignment, std::size_t newSize) noexcept
	{
		return nullptr;
	}

	void* AlignedECalloc(void* ptr, std::size_t alignment, std::size_t newCount, std::size_t newSize) noexcept
	{
		return nullptr;
	}

	void* AlignedEZCalloc(void* ptr, std::size_t alignment, std::size_t newCount, std::size_t newSize) noexcept
	{
		return nullptr;
	}

	void AlignedFree(void* ptr, std::size_t alignment) noexcept
	{
		if constexpr (Common::c_IsSystemWindows)
			_aligned_free(ptr);
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
		return AlignedRMalloc(ptr, 16, newSize);
	}

	void* RZalloc(void* ptr, std::size_t newSize) noexcept
	{
		return AlignedRZalloc(ptr, 16, newSize);
	}

	void* RCalloc(void* ptr, std::size_t newCount, std::size_t newSize) noexcept
	{
		return AlignedRCalloc(ptr, 16, newCount, newSize);
	}

	void* RZCalloc(void* ptr, std::size_t newCount, std::size_t newSize) noexcept
	{
		return AlignedRZCalloc(ptr, 16, newCount, newSize);
	}

	void* EMalloc(void* ptr, std::size_t newSize) noexcept
	{
		return AlignedEMalloc(ptr, 16, newSize);
	}

	void* EZalloc(void* ptr, std::size_t newSize) noexcept
	{
		return AlignedEZalloc(ptr, 16, newSize);
	}

	void* ECalloc(void* ptr, std::size_t newCount, std::size_t newSize) noexcept
	{
		return AlignedECalloc(ptr, 16, newCount, newSize);
	}

	void* EZCalloc(void* ptr, std::size_t newCount, std::size_t newSize) noexcept
	{
		return AlignedEZCalloc(ptr, 16, newCount, newSize);
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

	void* AlignedRMalloc(void* ptr, size_t alignment, size_t newSize)
	{
		return Memory::AlignedRMalloc(ptr, alignment, newSize);
	}

	void* AlignedRZalloc(void* ptr, size_t alignment, size_t newSize)
	{
		return Memory::AlignedRZalloc(ptr, alignment, newSize);
	}

	void* AlignedRCalloc(void* ptr, size_t alignment, size_t newCount, size_t newSize)
	{
		return Memory::AlignedRCalloc(ptr, alignment, newCount, newSize);
	}

	void* AlignedRZCalloc(void* ptr, size_t alignment, size_t newCount, size_t newSize)
	{
		return Memory::AlignedRZCalloc(ptr, alignment, newCount, newSize);
	}

	void* AlignedEMalloc(void* ptr, size_t alignment, size_t newSize)
	{
		return Memory::AlignedEMalloc(ptr, alignment, newSize);
	}

	void* AlignedEZalloc(void* ptr, size_t alignment, size_t newSize)
	{
		return Memory::AlignedEZalloc(ptr, alignment, newSize);
	}

	void* AlignedECalloc(void* ptr, size_t alignment, size_t newCount, size_t newSize)
	{
		return Memory::AlignedECalloc(ptr, alignment, newCount, newSize);
	}

	void* AlignedEZCalloc(void* ptr, size_t alignment, size_t newCount, size_t newSize)
	{
		return Memory::AlignedEZCalloc(ptr, alignment, newCount, newSize);
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