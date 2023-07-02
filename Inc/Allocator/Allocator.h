#pragma once

#include "Concurrency/Mutex.h"
#include "SortedMatrix.h"

#include <cstddef>
#include <cstdint>

namespace Allocator
{
	using RSM = Concurrency::RecursiveSharedMutex;
	template <Concurrency::MutexC... Mutexes>
	using ScopedLock = Concurrency::ScopedLock<Mutexes...>;
	template <Concurrency::SharedMutexC... Mutexes>
	using ScopedSharedLock = Concurrency::ScopedSharedLock<Mutexes...>;

	void* AllocatePages(std::uint64_t pageCount, bool internal = true) noexcept;
	void  FreePages(void* addr, std::uint64_t pageCount, bool internal = true) noexcept;

	template <class T>
	struct PagedArrayAlloc
	{
	public:
		PagedArrayAlloc() noexcept;

		T*   AllocRow() noexcept;
		void FreeRow(T* row) noexcept;

		std::size_t RowSize() const noexcept { return m_RowSize; }

	private:
		std::size_t m_RowSize;
	};

	struct UsedRange
	{
	public:
		std::uintptr_t Address;
		std::size_t    Size;

		std::uintptr_t End() const noexcept { return Address + Size; }
	};

	struct Page
	{
	public:
		std::uintptr_t Address;
		std::size_t    Size;
		std::uint64_t  RefCount;

		std::uintptr_t End() const noexcept { return Address + Size; }
	};

	struct UsedTable
	{
	public:
		UsedTable() noexcept;

	public:
		std::size_t Total;

		SortedMatrix<UsedRange, PagedArrayAlloc<UsedRange>> Matrix;
	};

	enum class EPageTableType : std::uint8_t
	{
		Small = 0,
		Large
	};

	struct PageTable
	{
	public:
		PageTable(EPageTableType type, std::uint8_t alignment) noexcept;
		~PageTable() noexcept;

	public:
		EPageTableType Type;
		std::uint8_t   Alignment;

		RSM Mtx;

		UsedTable* Used;
		UsedTable* Free;

		SortedMatrix<Page, PagedArrayAlloc<Page>> Matrix;
	};

	struct DebugSettings
	{
	public:
		std::uint8_t PushMessages : 1 = 0;
	};

	struct DebugStat
	{
	public:
		std::size_t Count = 0;
		std::size_t Bytes = 0;
	};

	struct DebugStats
	{
	public:
		DebugStat Internal;
		DebugStat External;
		DebugStat Small;
		DebugStat Large;
	};

	struct State
	{
	public:
		RSM Mtx;

		std::size_t  PageSize;
		std::uint8_t PageAlign;

		std::size_t ArrayPages;
		std::size_t TablePages;

		DebugSettings DebugSettings;
		DebugStats    DebugStats;

		PageTable* PageTables[120];

	public:
		State();
		~State();
	};

	struct AllocInfo
	{
	public:
		std::uintptr_t Address;
		std::size_t    Size;

		PageTable*  Table;
		std::size_t Index;

	public:
		AllocInfo() noexcept;
		AllocInfo(std::uintptr_t address, std::size_t size, PageTable* table, std::size_t index) noexcept;
		AllocInfo(const AllocInfo& copy) noexcept;
		AllocInfo(AllocInfo&& move) noexcept;
		~AllocInfo() noexcept;

		AllocInfo& operator=(const AllocInfo& copy) noexcept;
		AllocInfo& operator=(AllocInfo&& move) noexcept;
	};

	State& GetState();

	void          GetDebugSettings(DebugSettings& settings);
	DebugSettings SetDebugSettings(const DebugSettings& settings);
	void          GetDebugStats(DebugStats& stats);

	std::uint8_t FastAlign(std::size_t alignment);
	std::size_t  CountedSize(std::size_t count, std::size_t size, std::uint8_t alignment);
	std::size_t  AllocSize(std::size_t size, std::uint8_t alignment);
	bool         ShouldAllocSmall(std::size_t allocSize);

	AllocInfo FindAlloc(std::uintptr_t address, std::uint8_t alignment);
	AllocInfo Allocate(std::size_t size, std::uint8_t alignment);
	AllocInfo AllocateSmall(std::uint16_t allocSize, std::uint8_t alignment);
	AllocInfo AllocateLarge(std::size_t size, std::uint8_t alignment);
	bool      NeedsResize(const AllocInfo& alloc, std::size_t newSize);
	bool      TryResizeAlloc(const AllocInfo& alloc, std::size_t newSize, AllocInfo* newAlloc = nullptr);
	void      Free(const AllocInfo& alloc);
	void      ZeroAlloc(const AllocInfo& alloc);
	void      ZeroAllocRange(const AllocInfo& alloc, std::size_t offset, std::size_t size);

	template <class T>
	PagedArrayAlloc<T>::PagedArrayAlloc() noexcept
	{
		State& state = GetState();
		m_RowSize    = (state.ArrayPages << state.PageAlign) / sizeof(T);
	}

	template <class T>
	T* PagedArrayAlloc<T>::AllocRow() noexcept
	{
		return static_cast<T*>(AllocatePages(GetState().ArrayPages));
	}

	template <class T>
	void PagedArrayAlloc<T>::FreeRow(T* row) noexcept
	{
		FreePages(row, GetState().ArrayPages);
	}
} // namespace Allocator