#include "Memory/Memory.h"
#include "Build.h"

#if BUILD_IS_SYSTEM_WINDOWS
	#include <cstdlib>
	#include <cstring>
#endif

namespace Memory
{
	void* AlignedMalloc(std::size_t alignment, std::size_t size)
	{
		if constexpr (Common::c_IsSystemWindows)
			return _aligned_malloc(size, alignment);
		else
			return nullptr;
	}

	void* AlignedZalloc(std::size_t alignment, std::size_t size)
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

	void* AlignedCalloc(std::size_t alignment, std::size_t count, std::size_t size)
	{
		if constexpr (Common::c_IsSystemWindows)
			return AlignedMalloc(alignment, count * AlignCeil(size, alignment));
		else
			return nullptr;
	}

	void* AlignedZCalloc(std::size_t alignment, std::size_t count, std::size_t size)
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

	void* AlignedRMalloc(void* ptr, std::size_t alignment, std::size_t newSize)
	{
		if constexpr (Common::c_IsSystemWindows)
			return _aligned_realloc(ptr, newSize, alignment);
		else
			return nullptr;
	}

	void* AlignedRZalloc(void* ptr, std::size_t alignment, std::size_t newSize)
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

	void* AlignedRCalloc(void* ptr, std::size_t alignment, std::size_t newCount, std::size_t newSize)
	{
		if constexpr (Common::c_IsSystemWindows)
			return AlignedRMalloc(ptr, alignment, newCount * AlignCeil(newSize, alignment));
		else
			return nullptr;
	}

	void* AlignedRZCalloc(void* ptr, std::size_t alignment, std::size_t newCount, std::size_t newSize)
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

	void* AlignedEMalloc(void* ptr, std::size_t alignment, std::size_t newSize)
	{
		return nullptr;
	}

	void* AlignedEZalloc(void* ptr, std::size_t alignment, std::size_t newSize)
	{
		return nullptr;
	}

	void* AlignedECalloc(void* ptr, std::size_t alignment, std::size_t newCount, std::size_t newSize)
	{
		return nullptr;
	}

	void* AlignedEZCalloc(void* ptr, std::size_t alignment, std::size_t newCount, std::size_t newSize)
	{
		return nullptr;
	}

	void AlignedFree(void* ptr, std::size_t alignment)
	{
		if constexpr (Common::c_IsSystemWindows)
			_aligned_free(ptr);
	}

	void* Malloc(std::size_t size)
	{
		return AlignedMalloc(16, size);
	}

	void* Zalloc(std::size_t size)
	{
		return AlignedZalloc(16, size);
	}

	void* Calloc(std::size_t count, std::size_t size)
	{
		return AlignedCalloc(16, count, size);
	}

	void* ZCalloc(std::size_t count, std::size_t size)
	{
		return AlignedZCalloc(16, count, size);
	}

	void* RMalloc(void* ptr, std::size_t newSize)
	{
		return AlignedRMalloc(ptr, 16, newSize);
	}

	void* RZalloc(void* ptr, std::size_t newSize)
	{
		return AlignedRZalloc(ptr, 16, newSize);
	}

	void* RCalloc(void* ptr, std::size_t newCount, std::size_t newSize)
	{
		return AlignedRCalloc(ptr, 16, newCount, newSize);
	}

	void* RZCalloc(void* ptr, std::size_t newCount, std::size_t newSize)
	{
		return AlignedRZCalloc(ptr, 16, newCount, newSize);
	}

	void* EMalloc(void* ptr, std::size_t newSize)
	{
		return AlignedEMalloc(ptr, 16, newSize);
	}

	void* EZalloc(void* ptr, std::size_t newSize)
	{
		return AlignedEZalloc(ptr, 16, newSize);
	}

	void* ECalloc(void* ptr, std::size_t newCount, std::size_t newSize)
	{
		return AlignedECalloc(ptr, 16, newCount, newSize);
	}

	void* EZCalloc(void* ptr, std::size_t newCount, std::size_t newSize)
	{
		return AlignedEZCalloc(ptr, 16, newCount, newSize);
	}

	void Free(void* ptr)
	{
		return AlignedFree(ptr, 16);
	}
} // namespace Memory