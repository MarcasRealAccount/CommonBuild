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

	void* AllocatePages(std::uint64_t pageCount, bool internal) noexcept
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

	void FreePages(void* address, std::uint64_t pageCount, bool internal) noexcept
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
	static std::int32_t ValueComp(T a, T b) noexcept
	{
		return a < b ? -1 : (a > b ? 1 : 0);
	}

	template <class T>
	static std::size_t MaxMatrixRows() noexcept
	{
		constexpr std::size_t ConstSize = sizeof(T) - sizeof(void*);

		State& state = GetState();
		return ((state.TablePages << state.PageAlign) - ConstSize) / sizeof(void*);
	}

	UsedTable::UsedTable() noexcept
		: Total(0),
		  Matrix(MaxMatrixRows<UsedTable>())
	{
	}

	PageTable::PageTable(EPageTableType type, std::uint8_t alignment) noexcept
		: Type(type),
		  Alignment(alignment),
		  Used(nullptr),
		  Free(nullptr),
		  Matrix(MaxMatrixRows<PageTable>())
	{
		switch (Type)
		{
		case EPageTableType::Small:
		{
			State& state = GetState();
			Used         = static_cast<UsedTable*>(AllocatePages(state.TablePages));
			Free         = static_cast<UsedTable*>(AllocatePages(state.TablePages));
			new (Used) UsedTable();
			new (Free) UsedTable();
			break;
		}
		default:
			break;
		}
	}

	PageTable::~PageTable() noexcept
	{
		switch (Type)
		{
		case EPageTableType::Small:
		{
			State& state = GetState();
			FreePages(Used, state.TablePages);
			FreePages(Free, state.TablePages);
			Used = nullptr;
			Free = nullptr;
			break;
		}
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

		PageAlign = static_cast<std::uint8_t>(std::countr_zero(PageSize));

		ArrayPages = Memory::AlignCountCeil(1ULL << 20, PageSize);
		TablePages = Memory::AlignCountCeil(1ULL << 16, PageSize);

		PageTable* basePageTables = static_cast<PageTable*>(AllocatePages(120 * TablePages));
		for (std::uint8_t alignment = 4; alignment < 64; ++alignment)
		{
			for (std::uint8_t type = 0; type < 2; ++type)
			{
				std::uint8_t offset = alignment * 2 - 8 + type;
				PageTable*   table  = reinterpret_cast<PageTable*>(reinterpret_cast<std::uintptr_t>(basePageTables) + offset * (TablePages << PageAlign));
				new (table) PageTable(static_cast<EPageTableType>(type), alignment);
				PageTables[offset] = table;
			}
		}
	}

	State::~State()
	{
		auto lock = ScopedLock<RSM> { Mtx };

		// TODO(MarcasRealAccount): Push messages if there are unfreed user allocations
		for (std::uint8_t offset = 0; offset < 120; ++offset)
		{
			PageTable* table = PageTables[offset];
			table->~PageTable();
		}

		FreePages(PageTables[0], 120 * TablePages);
		for (std::uint8_t offset = 0; offset < 120; ++offset)
			PageTables[offset] = nullptr;
	}

	AllocInfo::AllocInfo() noexcept
		: Address(0),
		  Size(0),
		  Table(nullptr),
		  Index(0)
	{
	}

	AllocInfo::AllocInfo(std::uintptr_t address, std::size_t size, PageTable* table, std::size_t index) noexcept
		: Address(address),
		  Size(size),
		  Table(table),
		  Index(index)
	{
		if (Table)
			Table->Mtx.LockShared();
	}

	AllocInfo::AllocInfo(const AllocInfo& copy) noexcept
		: Address(copy.Address),
		  Size(copy.Size),
		  Table(copy.Table),
		  Index(copy.Index)
	{
		if (Table)
			Table->Mtx.LockShared();
	}

	AllocInfo::AllocInfo(AllocInfo&& move) noexcept
		: Address(move.Address),
		  Size(move.Size),
		  Table(move.Table),
		  Index(move.Index)
	{
		move.Address = 0;
		move.Size    = 0;
		move.Table   = nullptr;
		move.Index   = 0;
	}

	AllocInfo::~AllocInfo() noexcept
	{
		if (Table)
			Table->Mtx.UnlockShared();
		Table = nullptr;
	}

	AllocInfo& AllocInfo::operator=(const AllocInfo& copy) noexcept
	{
		if (Table)
			Table->Mtx.UnlockShared();
		Address = copy.Address;
		Size    = copy.Size;
		Table   = copy.Table;
		Index   = copy.Index;
		if (Table)
			Table->Mtx.LockShared();
		return *this;
	}

	AllocInfo& AllocInfo::operator=(AllocInfo&& move) noexcept
	{
		Address = move.Address;
		Size    = move.Size;
		Table   = move.Table;
		Index   = move.Index;

		move.Address = 0;
		move.Size    = 0;
		move.Table   = nullptr;
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

	static bool FindAlloc(PageTable* table, std::uintptr_t address, AllocInfo& info)
	{
		State& state = s_State;

		switch (table->Type)
		{
		case EPageTableType::Small:
		{
			std::size_t index = FindLessEqual(
				table->Used->Matrix,
				address,
				[](const UsedRange& range) -> std::uintptr_t { return range.Address; },
				ValueComp<std::uintptr_t>);

			UsedRange& range = table->Used->Matrix[index];
			if (address >= range.Address && address < range.End())
			{
				info = AllocInfo(range.Address, range.Size, table, index);
				return true;
			}
			return false;
		}
		case EPageTableType::Large:
		{
			std::size_t index = FindLessEqual(
				table->Matrix,
				address,
				[](const Page& page) -> std::uintptr_t { return page.Address; },
				ValueComp<std::uintptr_t>);

			Page& page = table->Matrix[index];
			if (address >= page.Address && address < page.End())
			{
				info = AllocInfo(page.Address, page.Size, table, index);
				return true;
			}
			return false;
		}
		default:
			return false;
		}
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
				if (FindAlloc(state.PageTables[i], address, info))
					return info;
			}
		}
		else
		{
			std::size_t idx0 = alignment * 2 - 8;
			std::size_t idx1 = idx0 + 1;
			if (FindAlloc(state.PageTables[idx1], address, info))
				return info;
			if (FindAlloc(state.PageTables[idx0], address, info))
				return info;
			for (std::uint8_t i = 0; i < 120; ++i)
			{
				if (i == idx0 || i == idx1)
					continue;
				if (FindAlloc(state.PageTables[i], address, info))
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
			return AllocateSmall(allocSize << alignment, alignment);
		else
			return AllocateLarge(size, alignment);
	}

	AllocInfo AllocateSmall(std::size_t allocSize, std::uint8_t alignment)
	{
		if (alignment < 4 || alignment > 63 || !allocSize)
			return {};

		State& state = s_State;
		auto   lock  = ScopedLock<RSM> { state.Mtx };

		PageTable*  table     = state.PageTables[alignment * 2 - 8];
		std::size_t freeIndex = FindGreaterEqual(
			table->Free->Matrix,
			allocSize,
			[](const UsedRange& range) -> std::size_t { return range.Size; },
			ValueComp<std::size_t>);
		UsedRange free = table->Free->Matrix[freeIndex];
		if (free.Size >= allocSize)
		{
			table->Free->Matrix.Erase(freeIndex);
			if (free.Size > allocSize)
			{
				table->Free->Matrix.Insert(
					UsedRange {
						.Address = free.Address + allocSize,
						.Size    = free.Size - allocSize },
					[](const auto& matrix, const UsedRange& range) -> std::size_t {
						return FindGreaterEqual(
							matrix,
							range.Size,
							[](const UsedRange& range) -> std::size_t { return range.Size; },
							ValueComp<std::size_t>);
					});
			}

			std::size_t index = table->Used->Matrix.Insert(
				UsedRange { free.Address, allocSize },
				[](const auto& matrix, const UsedRange& range) -> std::size_t {
					return FindGreaterEqual(
						matrix,
						range.Address,
						[](const UsedRange& range) -> std::uintptr_t { return range.Address; },
						ValueComp<std::uintptr_t>);
				});
			std::size_t pageIndex = FindLessEqual(
				table->Matrix,
				free.Address,
				[](const Page& page) -> std::uintptr_t { return page.Address; },
				ValueComp<std::uintptr_t>);
			Page& page = table->Matrix[pageIndex];
			if (free.Address >= page.Address && free.Address < page.End())
				++page.RefCount;
			return AllocInfo(free.Address, allocSize, table, index);
		}

		std::size_t    pages = Memory::AlignCountCeil(table->Matrix.Size() << 20, state.PageAlign);
		void*          addr  = AllocatePages(pages, false);
		std::uintptr_t addri = reinterpret_cast<std::uintptr_t>(addr);
		std::size_t    size  = pages << state.PageAlign;

		std::size_t index = table->Matrix.Insert(
			Page {
				.Address  = addri,
				.Size     = size,
				.RefCount = 1 },
			[](const auto& matrix, const Page& page) -> std::size_t {
				return FindGreaterEqual(
					matrix,
					page.Address,
					[](const Page& page) -> std::uintptr_t { return page.Address; },
					ValueComp<std::uintptr_t>);
			});

		std::size_t sizeLeft = size - allocSize;

		table->Free->Matrix.Insert(
			UsedRange {
				.Address = addri + sizeLeft,
				.Size    = sizeLeft },
			[](const auto& matrix, const UsedRange& range) -> std::size_t {
				return FindGreaterEqual(
					matrix,
					range.Size,
					[](const UsedRange& range) -> std::size_t { return range.Size; },
					ValueComp<std::size_t>);
			});
		std::size_t usedIndex = table->Used->Matrix.Insert(
			UsedRange {
				.Address = addri,
				.Size    = allocSize },
			[](const auto& matrix, const UsedRange& range) -> std::size_t {
				return FindGreaterEqual(
					matrix,
					range.Address,
					[](const UsedRange& range) -> std::uintptr_t { return range.Address; },
					ValueComp<std::uintptr_t>);
			});
		return AllocInfo(addri, allocSize, table, usedIndex);
	}

	AllocInfo AllocateLarge(std::size_t size, std::uint8_t alignment)
	{
		if (alignment < 4 || alignment > 63 || !size)
			return {};

		State& state = s_State;
		auto   lock  = ScopedLock<RSM> { state.Mtx };

		std::size_t    pages = Memory::AlignCountCeil(size, state.PageAlign);
		void*          addr  = AllocatePages(pages, false);
		std::uintptr_t addri = reinterpret_cast<std::uintptr_t>(addr);
		std::size_t    size  = pages << state.PageAlign;

		PageTable*  table = state.PageTables[alignment * 2 - 7];
		std::size_t index = table->Matrix.Insert(
			Page {
				.Address  = addri,
				.Size     = size,
				.RefCount = 1 },
			[](const auto& matrix, const Page& page) -> std::size_t {
				return FindGreaterEqual(
					matrix,
					page.Address,
					[](const Page& page) -> std::uintptr_t { return page.Address; },
					ValueComp<std::uintptr_t>);
			});

		return AllocInfo(addri, size, table, index);
	}

	bool NeedsResize(const AllocInfo& alloc, std::size_t newSize)
	{
		return newSize < (alloc.Size >> 1) || newSize > alloc.Size;
	}

	bool TryResizeAlloc(const AllocInfo& alloc, std::size_t newSize, AllocInfo* newAlloc)
	{
		return false;
	}

	void Free(const AllocInfo& alloc)
	{
		if (!alloc.Table)
			return;

		State& state = s_State;
		auto   lock  = ScopedLock<RSM> { state.Mtx };

		PageTable* table = alloc.Table;
		switch (table->Type)
		{
		case EPageTableType::Small:
		{
			UsedRange used = table->Used->Matrix[alloc.Index];
			table->Used->Matrix.Erase(alloc.Index);

			bool        insertFree = true;
			std::size_t pageIndex  = FindLessEqual(
                table->Matrix,
                alloc.Address,
                [](const Page& page) -> std::uintptr_t { return page.Address; },
                ValueComp<std::uintptr_t>);
			Page& page = table->Matrix[pageIndex];
			if (alloc.Address >= page.Address && alloc.Address < page.End())
			{
				if (--page.RefCount == 0)
				{
					FreePages(reinterpret_cast<void*>(page.Address), page.Size >> state.PageAlign, false);
					table->Matrix.Erase(pageIndex);
					insertFree = false;

					table->Free->Matrix.EraseIf([&page](const UsedRange& range) -> bool {
						return range.Address >= page.Address && range.Address < page.End();
					});
				}
			}

			if (insertFree)
			{
				std::uintptr_t newFreeAddr = used.Address;
				std::size_t    newFreeSize = used.Size;
				// TODO(MarcasRealAccount): Maybe look for both at the same time?
				std::size_t prevFree = FindEqualIter(
					table->Free->Matrix,
					used.Address,
					[](const UsedRange& range) -> std::uintptr_t { return range.End(); },
					ValueComp<std::uintptr_t>);
				if (prevFree < table->Free->Matrix.Size())
				{
					UsedRange& prev = table->Free->Matrix[prevFree];
					newFreeAddr     = prev.Address;
					newFreeSize    += prev.Size;
					table->Free->Matrix.Erase(prevFree);
				}

				std::size_t nextFree = FindEqualIter(
					table->Free->Matrix,
					used.End(),
					[](const UsedRange& range) -> std::uintptr_t { return range.Address; },
					ValueComp<std::uintptr_t>);
				if (nextFree < table->Free->Matrix.Size())
				{
					UsedRange& next = table->Free->Matrix[nextFree];
					newFreeSize    += next.Size;
					table->Free->Matrix.Erase(nextFree);
				}

				table->Free->Matrix.Insert(
					UsedRange { .Address = newFreeAddr,
								.Size    = newFreeSize },
					[](const auto& matrix, const UsedRange& range) -> std::size_t {
						return FindGreaterEqual(
							matrix,
							range.Size,
							[](const UsedRange& range) -> std::size_t { return range.Size; },
							ValueComp<std::size_t>);
					});
			}
			break;
		}
		case EPageTableType::Large:
		{
			auto& page = table->Matrix[alloc.Index];
			FreePages(reinterpret_cast<void*>(page.Address), page.Size >> state.PageAlign, false);
			table->Matrix.Erase(alloc.Index);
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
		std::size_t toSet = std::min<std::size_t>(alloc.Size - offset, size);
		std::memset(reinterpret_cast<std::uint8_t*>(alloc.Address) + offset, 0, toSet);
	}
} // namespace Allocator