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
		// TODO(MarcasRealAccount): Push message
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
		// TODO(MarcasRealAccount): Push message
	}

	template <class T>
	static std::int32_t ValueComp(T a, T b)
	{
		return a < b ? -1 : (a > b ? 1 : 0);
	}

	template <class T, class U, class Compare>
	static std::pair<std::size_t, std::size_t> BinarySearch(T** table, const U& value, std::size_t arraySize, std::size_t min, std::size_t max, Compare comp)
	{
		while (min != max)
		{
			std::size_t mid = min + ((max - min) >> 1);
			std::size_t i   = mid / arraySize;
			std::size_t j   = mid % arraySize;
			if (comp(table[i][j], value) < 0)
				min = mid + 1;
			else
				max = mid;
		}
		return {
			min / arraySize,
			min % arraySize
		};
	}

	template <class T, class U, class Compare>
	static std::pair<std::size_t, std::size_t> BinarySearchLess(T** table, const U& value, std::size_t arraySize, std::size_t min, std::size_t max, Compare comp)
	{
		if (min == max)
		{
			return {
				min / arraySize,
				max % arraySize
			};
		}

		max = std::max(min, max - 1);
		while (min != max)
		{
			std::size_t mid = min + ((max - min + 1) >> 1);
			std::size_t i   = mid / arraySize;
			std::size_t j   = mid % arraySize;
			if (comp(table[i][j], value) > 0)
				max = mid - 1;
			else
				min = mid;
		}
		return {
			min / arraySize,
			min % arraySize
		};
	}

	template <class T, class U, class Compare>
	static std::pair<std::size_t, std::size_t> Search(T** table, const U& value, std::size_t arraySize, std::size_t min, std::size_t max, Compare comp)
	{
		std::size_t ii = min / arraySize;
		std::size_t ij = min % arraySize;
		for (std::size_t i = min; i < max; ++i)
		{
			if (comp(table[ii][ij], value) == 0)
				break;
			if (++ij > arraySize)
			{
				++ii;
				ij = 0;
			}
		}
		return { ii, ij };
	}

	template <class T, class Compare>
	static std::pair<std::size_t, std::size_t> ReOrder(T** table, std::size_t index, std::size_t arraySize, std::size_t min, std::size_t max, Compare comp)
	{
		std::size_t i = index / arraySize;
		std::size_t j = index % arraySize;

		T original = std::move(table[i][j]);

		auto [k, l]         = BinarySearch<T, T, Compare>(table, original, arraySize, min, max, comp);
		std::size_t m       = k * arraySize + l;
		std::size_t moveMin = std::min(index + 1, m);
		std::size_t moveMax = std::max(index, m);
		std::size_t to      = index < m ? index : m + 1;
		Move(table, arraySize, moveMin, moveMax, to);

		table[k][l] = std::move(original);
		return { k, l };
	}

	enum class EIterateStatus
	{
		Continue = 0,
		Break
	};

	template <class T, class F>
	static EIterateStatus Iterate(T** table, std::size_t arraySize, std::size_t min, std::size_t max, F func)
	{
		EIterateStatus status = EIterateStatus::Continue;

		std::size_t ii = min / arraySize;
		std::size_t ij = min % arraySize;
		for (std::size_t i = min; i < max && status == EIterateStatus::Continue; ++i)
		{
			auto res = func(table[ii][ij], std::pair<std::size_t, std::size_t> { ii, ij });
			if constexpr (std::same_as<decltype(res), EIterateStatus>)
				status = res;
			if (++ij >= arraySize)
			{
				++ii;
				ij = 0;
			}
		}

		return status;
	}

	template <class T>
	static void MoveBackward(T** table, std::size_t arraySize, std::size_t min, std::size_t max, std::size_t to)
	{
		using ssize_t = std::make_signed_t<std::size_t>;

		std::size_t size = max - min;
		std::size_t j    = to + size;
		ssize_t     ii   = max / arraySize;
		ssize_t     ij   = max % arraySize;
		ssize_t     ji   = j / arraySize;
		ssize_t     jj   = j % arraySize;
		for (std::size_t i = max; i > min; --i)
		{
			table[ji][jj] = table[ii][ij];

			if (--ij < 0)
			{
				--ii;
				ij = arraySize - 1;
			}
			if (--jj < 0)
			{
				--ji;
				jj = arraySize - 1;
			}
		}
	}

	template <class T>
	static void MoveForward(T** table, std::size_t arraySize, std::size_t min, std::size_t max, std::size_t to)
	{
		std::size_t ii = min / arraySize;
		std::size_t ij = min % arraySize;
		std::size_t ji = to / arraySize;
		std::size_t jj = to % arraySize;
		for (std::size_t i = min; i < max; ++i)
		{
			table[ji][jj] = table[ii][ij];
			if (++ij >= arraySize)
			{
				++ii;
				ij = 0;
			}
			if (++jj >= arraySize)
			{
				++ji;
				jj = 0;
			}
		}
	}

	template <class T>
	static void Move(T** table, std::size_t arraySize, std::size_t min, std::size_t max, std::size_t to)
	{
		if (to < min)
			return MoveForward<T>(table, arraySize, min, max, to);
		else if (to > min)
			return MoveBackward<T>(table, arraySize, min, max, to);
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

		PageAlign = static_cast<std::uint8_t>(std::countr_zero(PageSize));

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
				RangeTable*  table = RangeTables[offset] = reinterpret_cast<RangeTable*>(reinterpret_cast<std::uint8_t*>(baseRangeTable) + offset * (TablePages << PageAlign));

				table->Header.Type      = static_cast<ERangeTableType>(type);
				table->Header.Alignment = alignment;
			}
		}
	}

	State::~State()
	{
		auto lock = ScopedLock<RSM> { Mtx };

		// TODO(MarcasRealAccount): Push messages if there are unfreed user allocations
		for (std::uint8_t offset = 0; offset < 120; ++offset)
		{
			RangeTable* table = RangeTables[offset];
			{
				auto lock2 = ScopedLock<RSM> { table->Header.Mtx };
				Iterate(table->Ranges, RangesPerArray, 0, table->Header.Last, [this](Range& range, [[maybe_unused]] std::pair<std::size_t, std::size_t> index) -> EIterateStatus {
					FreePages(reinterpret_cast<void*>(range.Address), range.Pages, false);
					for (std::size_t i = 0; i < range.UsedTable->Header.LastArray; ++i)
						FreePages(range.UsedTable->Used[i], ArrayPages);
					for (std::size_t i = 0; i < range.FreeTable->Header.LastArray; ++i)
						FreePages(range.FreeTable->Frees[i], ArrayPages);
					FreePages(range.UsedTable, 2 * TablePages);
					return EIterateStatus::Continue;
				});
			}
			for (std::size_t i = 0; i < table->Header.LastArray; ++i)
				FreePages(table->Ranges[i], ArrayPages);
		}

		FreePages(RangeTables[0], 120 * TablePages);
		for (std::uint8_t offset = 0; offset < 120; ++offset)
			RangeTables[offset] = nullptr;
	}

	RangeInfo::RangeInfo() noexcept
		: Table(nullptr),
		  Range(0)
	{
	}

	RangeInfo::RangeInfo(RangeTable* table, std::size_t range) noexcept
		: Table(table),
		  Range(range)
	{
		if (Table)
			Table->Header.Mtx.Lock();
	}

	RangeInfo::RangeInfo(const RangeInfo& copy) noexcept
		: Table(copy.Table),
		  Range(copy.Range)
	{
		if (Table)
			Table->Header.Mtx.Lock();
	}

	RangeInfo::RangeInfo(RangeInfo&& move) noexcept
		: Table(move.Table),
		  Range(move.Range)
	{
		move.Table = nullptr;
		move.Range = 0;
	}

	RangeInfo::~RangeInfo() noexcept
	{
		if (Table)
			Table->Header.Mtx.Unlock();
	}

	RangeInfo& RangeInfo::operator=(const RangeInfo& copy) noexcept
	{
		if (Table)
			Table->Header.Mtx.Unlock();
		Table = copy.Table;
		Range = copy.Range;
		if (Table)
			Table->Header.Mtx.Lock();
		return *this;
	}

	RangeInfo& RangeInfo::operator=(RangeInfo&& move) noexcept
	{
		if (Table)
			Table->Header.Mtx.Unlock();
		Table      = move.Table;
		Range      = move.Range;
		move.Table = nullptr;
		move.Range = 0;
		return *this;
	}

	AllocInfo::AllocInfo() noexcept
		: Address(0),
		  Size(0),
		  Index(0)
	{
	}

	AllocInfo::AllocInfo(std::uintptr_t address, std::size_t size, std::uint32_t index, RangeInfo range) noexcept
		: Address(address),
		  Size(size),
		  Index(index),
		  Range(std::move(range))
	{
		if (Address && Range.Table)
		{
			State&        state = s_State;
			std::uint64_t i     = Range.Range / state.RangeArraysPerTable;
			std::uint64_t j     = Range.Range % state.RangeArraysPerTable;

			auto* ranges = Range.Table->Ranges[i];
			auto& rng    = ranges[j];
			if (rng.UsedTable)
				rng.UsedTable->Header.Mtx.Lock();
			if (rng.FreeTable)
				rng.FreeTable->Header.Mtx.Lock();
		}
	}

	AllocInfo::AllocInfo(const AllocInfo& copy) noexcept
		: Address(copy.Address),
		  Size(copy.Size),
		  Index(copy.Index),
		  Range(copy.Range)
	{
		if (Address && Range.Table)
		{
			State&        state = s_State;
			std::uint64_t i     = Range.Range / state.RangeArraysPerTable;
			std::uint64_t j     = Range.Range % state.RangeArraysPerTable;

			auto* ranges = Range.Table->Ranges[i];
			auto& range  = ranges[j];
			if (range.UsedTable)
				range.UsedTable->Header.Mtx.Lock();
			if (range.FreeTable)
				range.FreeTable->Header.Mtx.Lock();
		}
	}

	AllocInfo::AllocInfo(AllocInfo&& move) noexcept
		: Address(move.Address),
		  Size(move.Size),
		  Index(move.Index),
		  Range(std::move(move.Range))
	{
		move.Address = 0;
		move.Size    = 0;
		move.Index   = 0;
	}

	AllocInfo::~AllocInfo() noexcept
	{
		if (Address && Range.Table)
		{
			State&        state = s_State;
			std::uint64_t i     = Range.Range / state.RangeArraysPerTable;
			std::uint64_t j     = Range.Range % state.RangeArraysPerTable;

			auto* ranges = Range.Table->Ranges[i];
			auto& range  = ranges[j];
			if (range.UsedTable)
				range.UsedTable->Header.Mtx.Unlock();
			if (range.FreeTable)
				range.FreeTable->Header.Mtx.Unlock();
		}
	}

	AllocInfo& AllocInfo::operator=(const AllocInfo& copy) noexcept
	{
		if (Address && Range.Table)
		{
			State&        state = s_State;
			std::uint64_t i     = Range.Range / state.RangeArraysPerTable;
			std::uint64_t j     = Range.Range % state.RangeArraysPerTable;

			auto* ranges = Range.Table->Ranges[i];
			auto& range  = ranges[j];
			if (range.UsedTable)
				range.UsedTable->Header.Mtx.Unlock();
			if (range.FreeTable)
				range.FreeTable->Header.Mtx.Unlock();
		}

		Address = copy.Address;
		Size    = copy.Size;
		Index   = copy.Index;
		Range   = copy.Range;

		if (Address && Range.Table)
		{
			State&        state = s_State;
			std::uint64_t i     = Range.Range / state.RangeArraysPerTable;
			std::uint64_t j     = Range.Range % state.RangeArraysPerTable;

			auto* ranges = Range.Table->Ranges[i];
			auto& range  = ranges[j];
			if (range.UsedTable)
				range.UsedTable->Header.Mtx.Lock();
			if (range.FreeTable)
				range.FreeTable->Header.Mtx.Lock();
		}

		return *this;
	}

	AllocInfo& AllocInfo::operator=(AllocInfo&& move) noexcept
	{
		if (Address && Range.Table)
		{
			State&        state = s_State;
			std::uint64_t i     = Range.Range / state.RangeArraysPerTable;
			std::uint64_t j     = Range.Range % state.RangeArraysPerTable;

			auto* ranges = Range.Table->Ranges[i];
			auto& range  = ranges[j];
			if (range.UsedTable)
				range.UsedTable->Header.Mtx.Unlock();
			if (range.FreeTable)
				range.FreeTable->Header.Mtx.Unlock();
		}

		Address = move.Address;
		Size    = move.Size;
		Index   = move.Index;
		Range   = std::move(move.Range);

		move.Address = 0;
		move.Size    = 0;
		move.Index   = 0;

		return *this;
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
		return static_cast<std::uint8_t>(std::countr_zero(alignment));
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

	static bool FindAlloc(RangeTable* rangeTable, Range& range, std::size_t rangeIndex, std::uint32_t element, AllocInfo& info)
	{
		State&     state = s_State;
		UsedTable* table = range.UsedTable;

		auto [i, j] = BinarySearchLess(table->Used,
									   element,
									   state.UsedPerArray,
									   0,
									   table->Header.Last,
									   [](UsedRange range, std::uint32_t element) {
										   return ValueComp(range.Start, element);
									   });

		UsedRange* pUsedRanges = table->Used[i];
		if (!pUsedRanges)
			return false;

		UsedRange& usedRange = pUsedRanges[j];
		if (element >= usedRange.Start && element <= usedRange.End)
		{
			info = {
				range.Address + (static_cast<std::size_t>(usedRange.Start) << rangeTable->Header.Alignment),
				usedRange.Size() << rangeTable->Header.Alignment,
				static_cast<std::uint32_t>(i * state.UsedPerArray + j),
				{rangeTable, rangeIndex}
			};
			return true;
		}
		return false;
	}

	static bool FindAlloc(RangeTable* table, std::uintptr_t address, AllocInfo& info)
	{
		State& state = s_State;

		auto [i, j] = BinarySearchLess(table->Ranges,
									   address,
									   state.RangesPerArray,
									   0,
									   table->Header.Last,
									   [](const Range& range, std::uintptr_t address) {
										   return ValueComp(range.Address, address);
									   });

		Range* pRanges = table->Ranges[i];
		if (!pRanges)
			return false;

		Range& range = pRanges[j];
		if (address >= range.Address && address < range.Address + (range.Pages << state.PageAlign))
		{
			switch (table->Header.Type)
			{
			case ERangeTableType::Small:
				if (FindAlloc(table, range, i * state.RangesPerArray + j, static_cast<std::uint32_t>((address - range.Address) >> table->Header.Alignment), info))
					return true;
				break;
			case ERangeTableType::Large:
				info = {
					range.Address,
					range.Pages << state.PageAlign,
					0,
					{table, i * state.RangesPerArray + j}
				};
				return true;
			}
		}
		return false;
	}

	AllocInfo FindAlloc(std::uintptr_t address, std::uint8_t alignment)
	{
		State& state = s_State;
		auto   lock  = ScopedSharedLock<RSM> { state.Mtx };
		// Check state.RangeTables[alignment * 2 - 7] and state.RangeTables[alignment * 2 - 8], then check all others
		// Essentially check the large and small range tables for the alignment first and then all other range tables

		AllocInfo info {};
		if (alignment < 4)
		{
			for (std::uint8_t i = 0; i < 120; ++i)
			{
				if (FindAlloc(state.RangeTables[i], address, info))
					return info;
			}
		}
		else
		{
			std::size_t idx0 = alignment * 2 - 8;
			std::size_t idx1 = idx0 + 1;
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
		}
		return info;
	}

	AllocInfo Allocate(std::size_t size, std::uint8_t alignment)
	{
		std::size_t allocSize = AllocSize(size, alignment);
		if (alignment < 4 || alignment > 63 || !allocSize)
			return {};

		if (ShouldAllocSmall(allocSize))
			return AllocateSmall(static_cast<std::uint16_t>(allocSize), alignment);
		else
			return AllocateLarge(size, alignment);
	}

	AllocInfo AllocateSmall(std::uint16_t allocSize, std::uint8_t alignment)
	{
		if (alignment < 4 || alignment > 63 || !allocSize)
			return {};

		State& state = s_State;
		auto   lock  = ScopedLock<RSM> { state.Mtx };

		AllocInfo info {};

		RangeTable*    table  = state.RangeTables[alignment * 2 - 8];
		EIterateStatus status = Iterate(table->Ranges, state.RangesPerArray, 0, table->Header.Last, [&](Range& range, std::pair<std::size_t, std::size_t> index) -> EIterateStatus {
			if (!range.FreeTable || !range.UsedTable || range.FreeTable->Header.Total < allocSize)
				return EIterateStatus::Continue;

			FreeTable* freeTable = range.FreeTable;
			UsedTable* usedTable = range.UsedTable;

			auto lock2 = ScopedLock<RSM, RSM> { freeTable->Header.Mtx, usedTable->Header.Mtx };

			auto [l, m] = BinarySearch(freeTable->Frees,
									   allocSize,
									   state.FreePerArray,
									   0,
									   freeTable->Header.Last,
									   [](FreeRange range, std::uint16_t allocSize) {
										   return ValueComp<std::size_t>(range.Size(), allocSize);
									   });

			FreeRange&  freeRange = freeTable->Frees[l][m];
			std::size_t rangeSize = freeRange.Size();
			if (rangeSize < allocSize)
				return EIterateStatus::Continue;

			std::uint32_t start      = freeRange.Start;
			std::uint32_t end        = start + allocSize - 1;
			freeTable->Header.Total -= allocSize;
			if (rangeSize > allocSize)
			{
				freeRange.Start += allocSize;

				auto [n, o] = ReOrder(freeTable->Frees,
									  l * state.FreePerArray + m,
									  state.FreePerArray,
									  0,
									  freeTable->Header.Last,
									  [](FreeRange range, FreeRange other) {
										  return ValueComp(range.Size(), other.Size());
									  });

				l = n;
				m = o;
			}
			else
			{
				freeRange.Start = 0;
				freeRange.End   = 0;
				std::size_t min = l * state.FreePerArray + m;
				Move(freeTable->Frees, state.FreePerArray, min + 1, freeTable->Header.Last, min);
				--freeTable->Header.Last;
			}

			auto [n, o] = BinarySearch(usedTable->Used,
									   start,
									   state.UsedPerArray,
									   0,
									   usedTable->Header.Last,
									   [](UsedRange range, std::uint32_t start) {
										   return ValueComp(range.Start, start);
									   });
			{
				if (usedTable->Header.Last == usedTable->Header.LastArray * state.UsedPerArray)
				{
					usedTable->Used[usedTable->Header.LastArray] = static_cast<UsedRange*>(AllocatePages(state.ArrayPages));
					++usedTable->Header.LastArray;
				}
				std::size_t min = n * state.UsedPerArray + o;
				Move(usedTable->Used, state.UsedPerArray, min, usedTable->Header.Last, min + 1);
			}
			UsedRange& usedRange     = usedTable->Used[n][o];
			usedRange                = { start, end };
			usedTable->Header.Total += allocSize;
			++usedTable->Header.Last;

			info = {
				range.Address + (static_cast<std::size_t>(start) << table->Header.Alignment),
				usedRange.Size() << table->Header.Alignment,
				static_cast<std::uint32_t>(n * state.UsedPerArray + o),
				{table, index.first * state.RangesPerArray + index.second}
			};
			auto& stat = state.DebugStats.Small;
			++stat.Count;
			stat.Bytes += info.Size;
			return EIterateStatus::Break;
		});
		if (status != EIterateStatus::Break)
		{
			std::size_t scale = 1;
			if (table->Header.Last > 0)
				scale = std::min<std::size_t>(2 * table->Header.Last, 128);
			std::size_t    maxElement = 65536 * scale;
			std::size_t    pageCount  = (maxElement << table->Header.Alignment) >> state.PageAlign;
			void*          addr       = AllocatePages(pageCount, false);
			std::uintptr_t address    = reinterpret_cast<std::uintptr_t>(addr);

			void* allocTables = AllocatePages(2 * state.TablePages);

			auto [i, j] = BinarySearch(table->Ranges,
									   address,
									   state.RangesPerArray,
									   0,
									   table->Header.Last,
									   [](Range& range, std::uintptr_t address) {
										   return ValueComp(range.Address, address);
									   });
			{
				if (table->Header.Last == table->Header.LastArray * state.RangesPerArray)
				{
					table->Ranges[table->Header.LastArray] = static_cast<Range*>(AllocatePages(state.ArrayPages));
					++table->Header.LastArray;
				}
				std::size_t min = i * state.RangesPerArray + j;
				Move(table->Ranges, state.RangesPerArray, min, table->Header.Last, min + 1);
			}
			Range& range = table->Ranges[i][j];
			range        = {
					   .Address   = address,
					   .Pages     = pageCount,
					   .UsedTable = static_cast<UsedTable*>(allocTables),
					   .FreeTable = reinterpret_cast<FreeTable*>(reinterpret_cast<std::uint8_t*>(allocTables) + (state.TablePages << state.PageAlign))
			};
			++table->Header.Last;

			UsedTable* usedTable = range.UsedTable;
			FreeTable* freeTable = range.FreeTable;

			usedTable->Header.Last      = 1;
			usedTable->Header.LastArray = 1;
			usedTable->Header.Total     = allocSize;
			usedTable->Used[0]          = static_cast<UsedRange*>(AllocatePages(state.ArrayPages));
			usedTable->Used[0][0]       = { 0, allocSize - 1U };
			freeTable->Header.Total     = maxElement - allocSize;
			if (freeTable->Header.Total > 0)
			{
				freeTable->Header.Last      = 1;
				freeTable->Header.LastArray = 1;
				freeTable->Frees[0]         = static_cast<FreeRange*>(AllocatePages(state.ArrayPages));
				freeTable->Frees[0][0]      = { allocSize, static_cast<std::uint32_t>(maxElement) - 1 };
			}
			else
			{
				freeTable->Header.Last      = 0;
				freeTable->Header.LastArray = 0;
			}

			info = {
				range.Address,
				static_cast<std::size_t>(allocSize) << table->Header.Alignment,
				0,
				{table, i * state.RangesPerArray + j}
			};
			auto& stat = state.DebugStats.Small;
			++stat.Count;
			stat.Bytes += info.Size;
		}

		return info;
	}

	AllocInfo AllocateLarge(std::size_t size, std::uint8_t alignment)
	{
		if (alignment < 4 || alignment > 63 || !size)
			return {};

		State& state = s_State;
		auto   lock  = ScopedLock<RSM> { state.Mtx };

		RangeTable* table = state.RangeTables[alignment * 2 - 7];

		std::size_t    pageCount = Memory::AlignCountCeil(size, 1ULL << state.PageAlign);
		void*          addr      = AllocatePages(pageCount, false);
		std::uintptr_t address   = reinterpret_cast<std::uintptr_t>(addr);

		auto [i, j] = BinarySearch(table->Ranges,
								   address,
								   state.RangesPerArray,
								   0,
								   table->Header.Last,
								   [](const Range& range, std::uintptr_t address) {
									   return ValueComp(range.Address, address);
								   });
		{
			if (table->Header.Last == table->Header.LastArray * state.RangesPerArray)
			{
				table->Ranges[table->Header.LastArray] = static_cast<Range*>(AllocatePages(state.ArrayPages));
				++table->Header.LastArray;
			}
			std::size_t min = i * state.RangesPerArray + j;
			Move(table->Ranges, state.RangesPerArray, min, table->Header.Last, min + 1);
		}
		Range& range = table->Ranges[i][j];
		range        = {
				   .Address   = address,
				   .Pages     = pageCount,
				   .UsedTable = nullptr,
				   .FreeTable = nullptr
		};
		++table->Header.Last;

		auto& stat = state.DebugStats.Large;
		++stat.Count;
		stat.Bytes += range.Pages << state.PageAlign;

		return {
			range.Address,
			range.Pages << state.PageAlign,
			0,
			{table, i * state.RangesPerArray + j}
		};
	}

	bool NeedsResize(const AllocInfo& alloc, std::size_t newSize)
	{
		if (!alloc.Address || !alloc.Range.Table)
			return false;

		std::size_t size = AllocSize(newSize, alloc.Range.Table->Header.Alignment);
		return size < (alloc.Size >> 1) || size > alloc.Size;
	}

	bool TryResizeAlloc(const AllocInfo& alloc, std::size_t newSize, AllocInfo* newAlloc)
	{
		if (!alloc.Address || !alloc.Range.Table)
			return false;

		// Only small allocations can be resized
		if (alloc.Range.Table->Header.Type != ERangeTableType::Small)
			return false;

		std::size_t newAllocSize = AllocSize(newSize, alloc.Range.Table->Header.Alignment);
		if (newAllocSize >= 65536)
			return false;

		State& state = s_State;

		std::size_t i = alloc.Range.Range / state.RangesPerArray;
		std::size_t j = alloc.Range.Range % state.RangesPerArray;

		Range& range = alloc.Range.Table->Ranges[i][j];

		auto [k, l] = Search(range.FreeTable->Frees,
							 alloc.Index + (alloc.Size >> alloc.Range.Table->Header.Alignment),
							 state.FreePerArray,
							 0,
							 range.FreeTable->Header.Last,
							 [](FreeRange range, std::uint32_t index) {
								 return ValueComp(range.Start, index);
							 });

		FreeRange&  freeRange = range.FreeTable->Frees[k][l];
		std::size_t allocSize = alloc.Size >> alloc.Range.Table->Header.Alignment;
		if (freeRange.Start != alloc.Index + allocSize)
			return false;

		std::size_t requiredSize = newAllocSize - allocSize;
		if (freeRange.Size() < requiredSize)
			return false;
		if (freeRange.Size() > requiredSize)
		{
			range.FreeTable->Header.Total -= requiredSize;
			freeRange.Start               += static_cast<std::uint32_t>(requiredSize);

			auto [m, n] = ReOrder(range.FreeTable->Frees,
								  k * state.FreePerArray + l,
								  state.FreePerArray,
								  0,
								  range.FreeTable->Header.Last,
								  [](FreeRange range, FreeRange other) {
									  return ValueComp(range.Size(), other.Size());
								  });

			k = m;
			l = n;
		}
		else
		{
			freeRange.Start = 0;
			freeRange.End   = 0;
			std::size_t min = k * state.FreePerArray + l;
			Move(range.FreeTable->Frees, state.FreePerArray, min + 1, range.FreeTable->Header.Last, min);
			--range.FreeTable->Header.Last;
		}

		std::size_t m                  = alloc.Index / state.UsedPerArray;
		std::size_t n                  = alloc.Index % state.UsedPerArray;
		UsedRange&  usedRange          = range.UsedTable->Used[m][n];
		usedRange.End                 += static_cast<std::uint32_t>(requiredSize);
		range.UsedTable->Header.Total += requiredSize;
		state.DebugStats.Small.Bytes  += requiredSize << alloc.Range.Table->Header.Alignment;
		if (newAlloc)
		{
			*newAlloc = {
				alloc.Address,
				newAllocSize << alloc.Range.Table->Header.Alignment,
				alloc.Index,
				{alloc.Range.Table, alloc.Range.Range}
			};
		}
		return true;
	}

	void Free(const AllocInfo& alloc)
	{
		if (!alloc.Address || !alloc.Range.Table)
			return;

		State& state = s_State;

		RangeTable* table = alloc.Range.Table;

		std::size_t i = alloc.Range.Range / state.RangesPerArray;
		std::size_t j = alloc.Range.Range % state.RangesPerArray;

		Range& range = table->Ranges[i][j];
		switch (table->Header.Type)
		{
		case ERangeTableType::Small:
		{
			std::size_t k         = alloc.Index / state.UsedPerArray;
			std::size_t l         = alloc.Index % state.UsedPerArray;
			UsedRange&  usedRange = range.UsedTable->Used[k][l];

			auto [m, n] = Search(range.FreeTable->Frees,
								 usedRange.Start - 1,
								 state.FreePerArray,
								 0,
								 range.FreeTable->Header.Last,
								 [](FreeRange range, std::uint32_t index) {
									 return ValueComp(range.End, index);
								 });

			auto [o, p] = Search(range.FreeTable->Frees,
								 usedRange.End + 1,
								 state.FreePerArray,
								 0,
								 range.FreeTable->Header.Last,
								 [](FreeRange range, std::uint32_t index) {
									 return ValueComp(range.Start, index);
								 });

			FreeRange& lowerFree = range.FreeTable->Frees[m][n];
			FreeRange& upperFree = range.FreeTable->Frees[o][p];

			std::size_t usedRangeSize = usedRange.Size();
			if (lowerFree.End == usedRange.Start - 1)
			{
				if (upperFree.Start == usedRange.End + 1)
				{
					lowerFree.End = upperFree.End;
					{
						upperFree.Start = 0;
						upperFree.End   = 0;
						std::size_t min = o * state.FreePerArray + p;
						Move(range.FreeTable->Frees, state.UsedPerArray, min + 1, range.FreeTable->Header.Last, min);
					}

					auto [q, r] = ReOrder(range.FreeTable->Frees,
										  m * state.FreePerArray + n,
										  state.FreePerArray,
										  0,
										  range.FreeTable->Header.Last,
										  [](FreeRange range, FreeRange other) {
											  return ValueComp(range.Size(), other.Size());
										  });
				}
				else
				{
					lowerFree.End = usedRange.End;

					auto [q, r] = ReOrder(range.FreeTable->Frees,
										  m * state.FreePerArray + n,
										  state.FreePerArray,
										  0,
										  range.FreeTable->Header.Last,
										  [](FreeRange range, FreeRange other) {
											  return ValueComp(range.Size(), other.Size());
										  });
				}
			}
			else
			{
				if (upperFree.Start == usedRange.End + 1)
				{
					upperFree.Start = usedRange.Start;

					auto [q, r] = ReOrder(range.FreeTable->Frees,
										  o * state.FreePerArray + p,
										  state.FreePerArray,
										  0,
										  range.FreeTable->Header.Last,
										  [](FreeRange range, FreeRange other) {
											  return ValueComp(range.Size(), other.Size());
										  });
				}
				else
				{
					auto [q, r] = BinarySearch(range.FreeTable->Frees,
											   usedRangeSize,
											   state.FreePerArray,
											   0,
											   range.FreeTable->Header.Last,
											   [](FreeRange range, std::size_t allocSize) {
												   return ValueComp(range.Size(), allocSize);
											   });
					{
						if (range.FreeTable->Header.Last == range.FreeTable->Header.LastArray * state.FreePerArray)
						{
							range.FreeTable->Frees[range.FreeTable->Header.LastArray] = static_cast<FreeRange*>(AllocatePages(state.ArrayPages));
							++range.FreeTable->Header.LastArray;
						}
						std::size_t min = q * state.FreePerArray + r;
						Move(range.FreeTable->Frees, state.FreePerArray, min, range.FreeTable->Header.Last, min + 1);
					}

					FreeRange& freeRange = range.FreeTable->Frees[q][r];
					freeRange.Start      = usedRange.Start;
					freeRange.End        = usedRange.End;
					++range.FreeTable->Header.Last;
				}
			}

			auto& stat = state.DebugStats.Small;
			--stat.Count;
			stat.Bytes -= alloc.Size;

			range.FreeTable->Header.Total += usedRangeSize;
			range.UsedTable->Header.Total -= usedRangeSize;
			std::size_t min                = alloc.Index;
			usedRange.Start                = 0;
			usedRange.End                  = 0;
			Move(range.UsedTable->Used, state.UsedPerArray, min + 1, range.UsedTable->Header.Last, min);
			--range.UsedTable->Header.Last;
			break;
		}
		case ERangeTableType::Large:
		{
			auto& stat = state.DebugStats.Large;
			--stat.Count;
			stat.Bytes -= range.Pages << state.PageAlign;

			FreePages(reinterpret_cast<void*>(range.Address), range.Pages, false);
			range.Address   = 0;
			range.Pages     = 0;
			std::size_t min = alloc.Range.Range;
			Move(table->Ranges, state.RangesPerArray, min + 1, table->Header.Last, min);
			--table->Header.Last;
			break;
		}
		}
	}

	void ZeroAlloc([[maybe_unused]] const AllocInfo& alloc)
	{
		ZeroAllocRange(alloc, 0, alloc.Size);
	}

	void ZeroAllocRange([[maybe_unused]] const AllocInfo& alloc, [[maybe_unused]] std::size_t offset, [[maybe_unused]] std::size_t size)
	{
		if (alloc.Range.Table->Header.Type == ERangeTableType::Small)
		{
			std::size_t toSet = std::min<std::size_t>(alloc.Size - offset, size);
			std::memset(reinterpret_cast<std::uint8_t*>(alloc.Address) + offset, 0, toSet);
			return;
		}
	}
} // namespace Allocator