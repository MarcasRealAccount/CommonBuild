#include "Memory/Memory.h"
#include "Allocator/Allocator.h"
#include "Build.h"

#include <bit>

namespace Memory
{
	static constexpr bool ToFastAlign(std::size_t alignment, std::uint8_t& fastAlign)
	{
		if (std::popcount(alignment) != 1)
			return false;
		fastAlign = Allocator::FastAlign(alignment);
		return true;
	}

	void* AlignedMalloc(std::size_t alignment, std::size_t size) noexcept
	{
		std::uint8_t fastAlign;
		if (!size || !ToFastAlign(alignment, fastAlign))
			return nullptr;

		Allocator::AllocInfo result = Allocator::Allocate(size, fastAlign);
		return reinterpret_cast<void*>(result.Address);
	}

	void* AlignedZalloc(std::size_t alignment, std::size_t size) noexcept
	{
		std::uint8_t fastAlign;
		if (!size || !ToFastAlign(alignment, fastAlign))
			return nullptr;

		Allocator::AllocInfo result = Allocator::Allocate(size, fastAlign);
		Allocator::ZeroAlloc(result);
		return reinterpret_cast<void*>(result.Address);
	}

	void* AlignedCalloc(std::size_t alignment, std::size_t count, std::size_t size) noexcept
	{
		std::uint8_t fastAlign;
		if (!size || !ToFastAlign(alignment, fastAlign))
			return nullptr;

		Allocator::AllocInfo result = Allocator::Allocate(Allocator::CountedSize(count, size, fastAlign), fastAlign);
		return reinterpret_cast<void*>(result.Address);
	}

	void* AlignedZCalloc(std::size_t alignment, std::size_t count, std::size_t size) noexcept
	{
		std::uint8_t fastAlign;
		if (!size || !ToFastAlign(alignment, fastAlign))
			return nullptr;

		Allocator::AllocInfo result = Allocator::Allocate(Allocator::CountedSize(count, size, fastAlign), fastAlign);
		Allocator::ZeroAlloc(result);
		return reinterpret_cast<void*>(result.Address);
	}

	void* AlignedRMalloc(void* ptr, std::size_t newSize, std::size_t alignment) noexcept
	{
		std::uint8_t fastAlign;
		if (!ToFastAlign(alignment, fastAlign))
			fastAlign = 0;

		Allocator::AllocInfo alloc = Allocator::FindAlloc(reinterpret_cast<std::uintptr_t>(ptr), fastAlign);
		if (!alloc.Address)
			return nullptr;

		if (!Allocator::NeedsResize(alloc, newSize))
			return reinterpret_cast<void*>(alloc.Address);

		{
			Allocator::AllocInfo newAlloc;
			if (Allocator::TryResizeAlloc(alloc, newSize, &newAlloc))
				return reinterpret_cast<void*>(newAlloc.Address);
		}

		Allocator::AllocInfo newAlloc = Allocator::Allocate(newSize, alloc.Range.Table->Header.Alignment);
		if (!newAlloc.Address)
			return nullptr;

		std::memcpy(reinterpret_cast<void*>(newAlloc.Address),
					reinterpret_cast<void*>(alloc.Address),
					std::min<std::size_t>(alloc.Size, newAlloc.Size));
		Allocator::Free(alloc);
		return reinterpret_cast<void*>(newAlloc.Address);
	}

	void* AlignedRZalloc(void* ptr, std::size_t newSize, std::size_t alignment) noexcept
	{
		std::uint8_t fastAlign;
		if (!ToFastAlign(alignment, fastAlign))
			fastAlign = 0;

		Allocator::AllocInfo alloc = Allocator::FindAlloc(reinterpret_cast<std::uintptr_t>(ptr), fastAlign);
		if (!alloc.Address)
			return nullptr;

		if (!Allocator::NeedsResize(alloc, newSize))
			return reinterpret_cast<void*>(alloc.Address);

		{
			Allocator::AllocInfo newAlloc;
			if (Allocator::TryResizeAlloc(alloc, newSize, &newAlloc))
			{
				if (newAlloc.Size > alloc.Size)
					Allocator::ZeroAllocRange(newAlloc, alloc.Size, newAlloc.Size - alloc.Size);
				return reinterpret_cast<void*>(newAlloc.Address);
			}
		}

		Allocator::AllocInfo newAlloc = Allocator::Allocate(newSize, alloc.Range.Table->Header.Alignment);
		if (!newAlloc.Address)
			return nullptr;

		std::memcpy(reinterpret_cast<void*>(newAlloc.Address),
					reinterpret_cast<void*>(alloc.Address),
					std::min<std::size_t>(alloc.Size, newAlloc.Size));
		if (newAlloc.Size > alloc.Size)
			Allocator::ZeroAllocRange(newAlloc, alloc.Size, newAlloc.Size - alloc.Size);
		Allocator::Free(alloc);
		return reinterpret_cast<void*>(newAlloc.Address);
	}

	void* AlignedRCalloc(void* ptr, std::size_t newCount, std::size_t newSize, std::size_t alignment) noexcept
	{
		std::uint8_t fastAlign;
		if (!ToFastAlign(alignment, fastAlign))
			fastAlign = 0;

		Allocator::AllocInfo alloc = Allocator::FindAlloc(reinterpret_cast<std::uintptr_t>(ptr), fastAlign);
		if (!alloc.Address)
			return nullptr;

		std::size_t newCountedSize = Allocator::CountedSize(newCount, newSize, alloc.Range.Table->Header.Alignment);

		if (!Allocator::NeedsResize(alloc, newCountedSize))
			return reinterpret_cast<void*>(alloc.Address);

		{
			Allocator::AllocInfo newAlloc;
			if (Allocator::TryResizeAlloc(alloc, newCountedSize, &newAlloc))
				return reinterpret_cast<void*>(newAlloc.Address);
		}

		Allocator::AllocInfo newAlloc = Allocator::Allocate(newCountedSize, alloc.Range.Table->Header.Alignment);
		if (!newAlloc.Address)
			return nullptr;

		std::memcpy(reinterpret_cast<void*>(newAlloc.Address),
					reinterpret_cast<void*>(alloc.Address),
					std::min<std::size_t>(alloc.Size, newAlloc.Size));
		Allocator::Free(alloc);
		return reinterpret_cast<void*>(newAlloc.Address);
	}

	void* AlignedRZCalloc(void* ptr, std::size_t newCount, std::size_t newSize, std::size_t alignment) noexcept
	{
		std::uint8_t fastAlign;
		if (!ToFastAlign(alignment, fastAlign))
			fastAlign = 0;

		Allocator::AllocInfo alloc = Allocator::FindAlloc(reinterpret_cast<std::uintptr_t>(ptr), fastAlign);
		if (!alloc.Address)
			return nullptr;

		std::size_t newCountedSize = Allocator::CountedSize(newCount, newSize, alloc.Range.Table->Header.Alignment);

		if (!Allocator::NeedsResize(alloc, newCountedSize))
			return reinterpret_cast<void*>(alloc.Address);

		{
			Allocator::AllocInfo newAlloc;
			if (Allocator::TryResizeAlloc(alloc, newCountedSize, &newAlloc))
			{
				if (newAlloc.Size > alloc.Size)
					Allocator::ZeroAllocRange(newAlloc, alloc.Size, newAlloc.Size - alloc.Size);
				return reinterpret_cast<void*>(newAlloc.Address);
			}
		}

		Allocator::AllocInfo newAlloc = Allocator::Allocate(newCountedSize, alloc.Range.Table->Header.Alignment);
		if (!newAlloc.Address)
			return nullptr;

		std::memcpy(reinterpret_cast<void*>(newAlloc.Address),
					reinterpret_cast<void*>(alloc.Address),
					std::min<std::size_t>(alloc.Size, newAlloc.Size));
		if (newAlloc.Size > alloc.Size)
			Allocator::ZeroAllocRange(newAlloc, alloc.Size, newAlloc.Size - alloc.Size);
		Allocator::Free(alloc);
		return reinterpret_cast<void*>(newAlloc.Address);
	}

	void* AlignedEMalloc(void* ptr, std::size_t newSize, std::size_t alignment) noexcept
	{
		std::uint8_t fastAlign;
		if (!ToFastAlign(alignment, fastAlign))
			fastAlign = 0;

		Allocator::AllocInfo alloc = Allocator::FindAlloc(reinterpret_cast<std::uintptr_t>(ptr), fastAlign);
		if (!alloc.Address)
			return nullptr;

		if (!Allocator::NeedsResize(alloc, newSize))
			return reinterpret_cast<void*>(alloc.Address);

		Allocator::AllocInfo newAlloc;
		if (Allocator::TryResizeAlloc(alloc, newSize, &newAlloc))
			return reinterpret_cast<void*>(newAlloc.Address);
		return nullptr;
	}

	void* AlignedEZalloc(void* ptr, std::size_t newSize, std::size_t alignment) noexcept
	{
		std::uint8_t fastAlign;
		if (!ToFastAlign(alignment, fastAlign))
			fastAlign = 0;

		Allocator::AllocInfo alloc = Allocator::FindAlloc(reinterpret_cast<std::uintptr_t>(ptr), fastAlign);
		if (!alloc.Address)
			return nullptr;

		if (!Allocator::NeedsResize(alloc, newSize))
			return reinterpret_cast<void*>(alloc.Address);

		Allocator::AllocInfo newAlloc;
		if (Allocator::TryResizeAlloc(alloc, newSize, &newAlloc))
		{
			if (newAlloc.Size > alloc.Size)
				Allocator::ZeroAllocRange(newAlloc, alloc.Size, newAlloc.Size - alloc.Size);
			return reinterpret_cast<void*>(newAlloc.Address);
		}
		return nullptr;
	}

	void* AlignedECalloc(void* ptr, std::size_t newCount, std::size_t newSize, std::size_t alignment) noexcept
	{
		std::uint8_t fastAlign;
		if (!ToFastAlign(alignment, fastAlign))
			fastAlign = 0;

		Allocator::AllocInfo alloc = Allocator::FindAlloc(reinterpret_cast<std::uintptr_t>(ptr), fastAlign);
		if (!alloc.Address)
			return nullptr;

		std::size_t newCountedSize = Allocator::CountedSize(newCount, newSize, alloc.Range.Table->Header.Alignment);

		if (!Allocator::NeedsResize(alloc, newCountedSize))
			return reinterpret_cast<void*>(alloc.Address);

		Allocator::AllocInfo newAlloc;
		if (Allocator::TryResizeAlloc(alloc, newCountedSize, &newAlloc))
			return reinterpret_cast<void*>(newAlloc.Address);
		return nullptr;
	}

	void* AlignedEZCalloc(void* ptr, std::size_t newCount, std::size_t newSize, std::size_t alignment) noexcept
	{
		std::uint8_t fastAlign;
		if (!ToFastAlign(alignment, fastAlign))
			fastAlign = 0;

		Allocator::AllocInfo alloc = Allocator::FindAlloc(reinterpret_cast<std::uintptr_t>(ptr), fastAlign);
		if (!alloc.Address)
			return nullptr;

		std::size_t newCountedSize = Allocator::CountedSize(newCount, newSize, alloc.Range.Table->Header.Alignment);

		if (!Allocator::NeedsResize(alloc, newCountedSize))
			return reinterpret_cast<void*>(alloc.Address);

		Allocator::AllocInfo newAlloc;
		if (Allocator::TryResizeAlloc(alloc, newCountedSize, &newAlloc))
		{
			if (newAlloc.Size > alloc.Size)
				Allocator::ZeroAllocRange(newAlloc, alloc.Size, newAlloc.Size - alloc.Size);
			return reinterpret_cast<void*>(newAlloc.Address);
		}
		return nullptr;
	}

	void AlignedFree(void* ptr, std::size_t alignment) noexcept
	{
		std::uint8_t fastAlign;
		if (!ToFastAlign(alignment, fastAlign))
			fastAlign = 0;

		Allocator::AllocInfo alloc = Allocator::FindAlloc(reinterpret_cast<std::uintptr_t>(ptr), fastAlign);
		if (!alloc.Address)
			return;
		Allocator::Free(alloc);
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