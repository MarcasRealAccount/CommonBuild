#include "Allocator/Allocator.h"
#include "Build.h"
#include "Concurrency/Mutex.h"
#include "Memory/Memory.h"

#if BUILD_IS_SYSTEM_WINDOWS
	#include <Windows.h>
#endif

#include <bit>

namespace Allocator
{
	static State s_State;

	static void* AllocatePages(std::uint64_t pageCount, bool internal = true)
	{
#if BUILD_IS_SYSTEM_WINDOWS
		void* ptr = VirtualAlloc(nullptr, pageCount << s_State.PageAlign, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#endif
		{
			auto& state = s_State;
			auto  lock  = ScopedLock<RSM> { state.Mtx };

			auto& stats = state.DebugStats;
			auto& stat  = internal ? stats.Internal : stats.External;
			++stat.Count;
			stat.Bytes += pageCount << s_State.PageAlign;
		}
		return ptr;
	}

	static void FreePages(void* address, std::uint64_t pageCount, bool internal = true)
	{
#if BUILD_IS_SYSTEM_WINDOWS
		bool res = VirtualFree(address, 0, MEM_RELEASE);
#endif
		if (res)
		{
			auto& state = s_State;
			auto  lock  = ScopedLock<RSM> { state.Mtx };

			auto& stats = state.DebugStats;
			auto& stat  = internal ? stats.Internal : stats.External;
			--stat.Count;
			stat.Bytes -= pageCount << s_State.PageAlign;
		}
	}

	State::State()
	{
		auto lock = ScopedLock<RSM> { Mtx };

#if BUILD_IS_SYSTEM_WINDOWS
		{
			SYSTEM_INFO sysInfo {};
			GetSystemInfo(&sysInfo);
			PageSize = sysInfo.dwAllocationGranularity;
		}
#endif

		PageAlign = std::countr_zero(PageSize);

		ArrayPages = Memory::AlignCountCeil(1ULL << 20, PageSize);
		TablePages = Memory::AlignCountCeil(1ULL << 16, PageSize);

		RangesPerArray      = (ArrayPages << PageAlign) / sizeof(Range);
		UsedPerArray        = (ArrayPages << PageAlign) / sizeof(UsedRange);
		FreePerArray        = (ArrayPages << PageAlign) / sizeof(FreeRange);
		RangeArraysPerTable = ((TablePages << PageAlign) - sizeof(RangeTableHeader)) / sizeof(Range*);
		UsedArraysPerTable  = ((TablePages << PageAlign) - sizeof(UsedTableHeader)) / sizeof(UsedRange*);
		FreeArraysPerTable  = ((TablePages << PageAlign) - sizeof(FreeTableHeader)) / sizeof(FreeRange*);

		RangeTable* baseRangeTable = reinterpret_cast<RangeTable*>(AllocatePages(120 * TablePages));
		for (std::uint8_t alignment = 4; alignment < 64; ++alignment)
		{
			for (std::uint8_t type = 0; type < 2; ++type)
			{
				std::uint8_t offset = (alignment - 4) * 2 + type;
				RangeTables[offset] = reinterpret_cast<RangeTable*>(reinterpret_cast<std::uint8_t*>(baseRangeTable) + offset * (TablePages << PageAlign));
			}
		}
	}

	State::~State()
	{
		auto lock = ScopedLock<RSM> { Mtx };

		FreePages(RangeTables[0], 120 * TablePages);
		for (std::uint8_t offset = 0; offset < 120; ++offset)
			RangeTables[offset] = nullptr;
	}

	State& GetState()
	{
		return s_State;
	}

	void GetDebugSettings(DebugSettings& settings)
	{
		State& state = s_State;
		auto   lock  = ScopedSharedLock<RSM> { state.Mtx };
		settings     = state.DebugSettings;
	}

	DebugSettings SetDebugSettings(const DebugSettings& settings)
	{
		State&        state = s_State;
		auto          lock  = ScopedSharedLock<RSM> { state.Mtx };
		DebugSettings copy  = state.DebugSettings;
		state.DebugSettings = settings;
		return copy;
	}

	void GetDebugStats(DebugStats& stats)
	{
		State& state = s_State;
		auto   lock  = ScopedSharedLock<RSM> { state.Mtx };
		stats        = state.DebugStats;
	}

	std::uint8_t FastAlign(std::size_t alignment)
	{
		return std::countr_zero(alignment);
	}

	std::size_t CountedSize(std::size_t count, std::size_t size, std::uint8_t alignment)
	{
		return count * Memory::AlignCeil(size, 1ULL << alignment);
	}

	std::size_t AllocSize(std::size_t size, std::uint8_t alignment)
	{
		return size >> alignment;
	}

	bool ShouldAllocSmall(std::size_t allocSize)
	{
		return allocSize < 65536;
	}

	static bool FindAlloc(UsedTable* table, std::uintptr_t address, std::uint8_t alignment, std::uint16_t element, AllocInfo& info)
	{
		State&      state = s_State;
		std::size_t i     = 0;
		{
			std::size_t j = table->Header.LastArray;
			while (i != j)
			{
				std::size_t mid = i + ((j - i) >> 1);
				if (table->Used[mid]->Start < element)
					i = mid + 1;
				else
					j = mid;
			}
		}

		UsedRange*  ranges = table->Used[i];
		std::size_t j      = 0;
		{
			std::size_t k = i == table->Header.LastArray
								? table->Header.Last - table->Header.LastArray * state.UsedPerArray
								: state.UsedPerArray;
			while (j != k)
			{
				std::size_t mid = j + ((k - j) >> 1);
				if (ranges[mid].Start < element)
					j = mid + 1;
				else
					k = mid;
			}
		}

		UsedRange& range = ranges[j];
		if (element >= range.Start && element <= range.End)
		{
			info.Address = address + range.Start << alignment;
			info.Size    = (static_cast<std::size_t>(range.End) - range.Start + 1) << alignment;
			info.Index   = i * state.UsedPerArray + j;
			return true;
		}
		return false;
	}

	static bool FindAlloc(RangeTable* table, std::uintptr_t address, AllocInfo& info)
	{
		State&      state = s_State;
		std::size_t i     = 0;
		{
			std::size_t j = table->Header.LastArray;
			while (i != j)
			{
				std::size_t mid = i + ((j - i) >> 1);
				if (table->Ranges[mid][0].Address < address)
					i = mid + 1;
				else
					j = mid;
			}
		}

		Range*      ranges = table->Ranges[i];
		std::size_t j      = 0;
		{
			std::size_t k = i == table->Header.LastArray
								? table->Header.Last - table->Header.LastArray * state.RangesPerArray
								: state.RangesPerArray;
			while (j != k)
			{
				std::size_t mid = j + ((k - j) >> 1);
				if (ranges[mid].Address < address)
					j = mid + 1;
				else
					k = mid;
			}
		}

		Range& range = ranges[j];
		if (address >= range.Address && address < range.Address + (range.Pages << state.PageAlign))
		{
			switch (table->Header.Type)
			{
			case ERangeTableType::Small:
				if (FindAlloc(range.UsedTable, range.Address, table->Header.Alignment, (address - range.Address) >> table->Header.Alignment, info))
				{
					info.Range = {
						.Table = table,
						.Range = i * state.RangesPerArray + j
					};
					return true;
				}
				break;
			case ERangeTableType::Large:
				info.Address = range.Address;
				info.Size    = range.Pages << state.PageAlign;
				info.Index   = 0;
				info.Range   = {
					  .Table = table,
					  .Range = i * state.RangesPerArray + j
				};
				return true;
			}
		}
		return false;
	}

	AllocInfo FindAlloc(std::uintptr_t address, std::uint8_t alignment)
	{
		if (alignment < 4)
			return {};

		State& state = s_State;
		auto   lock  = ScopedSharedLock<RSM> { state.Mtx };
		// Check state.RangeTables[alignment * 2 - 7] and state.RangeTables[alignment * 2 - 8], then check all others
		// Essentially check the large and small range tables for the alignment first and then all other range tables

		std::size_t idx0 = alignment * 2 - 8;
		std::size_t idx1 = idx0 + 1;
		AllocInfo   info {};
		if (FindAlloc(state.RangeTables[idx1], address, info))
			return info;
		if (FindAlloc(state.RangeTables[idx0], address, info))
			return info;
		for (std::uint8_t i = 0; i < 120; ++i)
		{
			if (i == idx0 || i == idx1)
				continue;
			if (FindAlloc(state.RangeTables[i], address, info))
				return info;
		}
		return info;
	}
} // namespace Allocator