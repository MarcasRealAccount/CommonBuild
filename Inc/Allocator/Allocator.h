#pragma once

#include "Concurrency/Mutex.h"

#include <cstddef>
#include <cstdint>

namespace Allocator
{
	using RSM = Concurrency::RecursiveSharedMutex;
	template <Concurrency::MutexC... Mutexes>
	using ScopedLock = Concurrency::ScopedLock<Mutexes...>;
	template <Concurrency::SharedMutexC... Mutexes>
	using ScopedSharedLock = Concurrency::ScopedSharedLock<Mutexes...>;

	struct UsedRange
	{
	public:
		std::uint16_t Start;
		std::uint16_t End;
	};

	struct FreeRange
	{
	public:
		std::uint16_t Start;
		std::uint16_t End;
	};

	struct UsedTableHeader
	{
	public:
		std::size_t Total;
		std::size_t Last;
		std::size_t LastArray;

		RSM Mtx;
	};

	struct UsedTable
	{
	public:
		UsedTableHeader Header;
		UsedRange*      Used[1];
	};

	struct FreeTableHeader
	{
	public:
		std::size_t Total;
		std::size_t Last;
		std::size_t LastArray;

		RSM Mtx;
	};

	struct FreeTable
	{
	public:
		FreeTableHeader Header;
		FreeRange*      Frees[1];
	};

	struct Range
	{
	public:
		std::uintptr_t Address;
		std::size_t    Pages;
		UsedTable*     UsedTable;
		FreeTable*     FreeTable;
	};

	enum class ERangeTableType : std::uint8_t
	{
		Small = 0,
		Large
	};

	struct RangeTableHeader
	{
	public:
		ERangeTableType Type;
		std::uint8_t    Alignment;
		std::size_t     Last;
		std::size_t     LastArray;

		RSM Mtx;
	};

	struct RangeTable
	{
	public:
		RangeTableHeader Header;
		Range*           Ranges[1];
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

		std::size_t RangesPerArray;
		std::size_t UsedPerArray;
		std::size_t FreePerArray;
		std::size_t RangeArraysPerTable;
		std::size_t UsedArraysPerTable;
		std::size_t FreeArraysPerTable;

		DebugSettings DebugSettings;
		DebugStats    DebugStats;

		RangeTable* RangeTables[120];

	public:
		State();
		~State();
	};

	struct RangeInfo
	{
	public:
		RangeTable* Table;
		std::size_t Range;
	};

	struct AllocInfo
	{
	public:
		std::uintptr_t Address;
		std::size_t    Size;
		std::size_t    Index;

		RangeInfo Range;
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
} // namespace Allocator