#include <Allocator/Allocator.h>
#include <Memory/Memory.h>
#include <Testing/Testing.h>

static void TestAlloc(void* (*alloc)(), void (*free)(void* data), void* (*realloc)(void* data) = nullptr, void (*test)(void* data) = nullptr)
{
	Allocator::DebugStats stats {};
	Allocator::GetDebugStats(stats);

	void* data = alloc();
	Testing::Assert(data);
	if (realloc)
	{
		void* newData = realloc(data);
		if (!newData)
			free(data);
		Testing::Assert(newData);
		data = newData;
	}
	if (test)
		test(data);
	free(data);

	Allocator::DebugStats stats2 {};
	Allocator::GetDebugStats(stats2);
	Testing::Assert(stats.Large.Count == stats2.Large.Count &&
					stats.Large.Bytes == stats2.Large.Bytes &&
					stats.Small.Count == stats2.Small.Count &&
					stats.Small.Bytes == stats2.Small.Bytes);
}

template <std::size_t N>
static void TestMultiAlloc(void* (*alloc)(), void (*free)(void* data), void* (*realloc)(void* data) = nullptr, void (*test)(void* data) = nullptr)
{
	Allocator::DebugStats stats {};
	Allocator::GetDebugStats(stats);

	void* datas[N];
	for (std::size_t i = 0; i < N; ++i)
		Testing::Assert(datas[i] = alloc());
	if (realloc)
	{
		for (std::size_t i = 0; i < N; ++i)
		{
			void* newData = realloc(datas[i]);
			if (!newData)
			{
				free(datas[i]);
				datas[i] = nullptr;
			}
			Testing::Assert(newData);
			datas[i] = newData;
		}
	}
	if (test)
	{
		for (std::size_t i = 0; i < N; ++i)
			test(datas[i]);
	}
	for (std::size_t i = 0; i < N; ++i)
		free(datas[i]);

	Allocator::DebugStats stats2 {};
	Allocator::GetDebugStats(stats2);
	Testing::Assert(stats.Large.Count == stats2.Large.Count &&
					stats.Large.Bytes == stats2.Large.Bytes &&
					stats.Small.Count == stats2.Small.Count &&
					stats.Small.Bytes == stats2.Small.Bytes);
}

static void TimedTestAlloc(void* (*alloc)(), void (*free)(void* data), void* (*realloc)(void* data) = nullptr)
{
	void* data = alloc();
	if (realloc)
	{
		void* newData = realloc(data);
		if (!newData)
			free(data);
		data = newData;
	}
	free(data);
}

template <std::size_t N>
static void TimedTestMultiAlloc(void* (*alloc)(), void (*free)(void* data), void* (*realloc)(void* data) = nullptr)
{
	void* datas[N];
	for (std::size_t i = 0; i < N; ++i)
		datas[i] = alloc();
	if (realloc)
	{
		for (std::size_t i = 0; i < N; ++i)
		{
			void* newData = realloc(datas[i]);
			if (!newData)
				free(datas[i]);
			datas[i] = newData;
		}
	}
	for (std::size_t i = 0; i < N; ++i)
		free(datas[i]);
}

void MemoryTests()
{
	Testing::PushGroup("Allocator Debugger");
	Testing::Test("GetDebugSettings", []() {
		Allocator::DebugSettings settings {};
		Allocator::GetDebugSettings(settings);
		Testing::Assert(!settings.PushMessages); // Assert on default value
	});
	Testing::Test("SetDebugSettings", []() {
		Allocator::DebugSettings settings {};
		settings.PushMessages = true;
		Allocator::SetDebugSettings(settings);
		Allocator::GetDebugSettings(settings);
		Testing::Assert(settings.PushMessages);
	});
	Testing::Test("GetDebugStats", []() {
		Allocator::DebugStats stats {};
		Allocator::GetDebugStats(stats);
	});
	Testing::PopGroup();

	auto basicTests = []() {
		Testing::Test("Malloc 1 KiB", []() { TestAlloc([]() { return Memory::Malloc(1024); }, [](void* data) { Memory::Free(data); }); });
		Testing::Test("RMalloc 1 KiB -> 2 KiB", []() { TestAlloc([]() { return Memory::Malloc(1024); }, [](void* data) { Memory::Free(data); }, [](void* data) { return Memory::RMalloc(data, 2048); }); });
		Testing::Test("Zalloc 1 KiB",
					  []() { TestAlloc([]() { return Memory::Zalloc(1024); },
									   [](void* data) { Memory::Free(data); },
									   nullptr,
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 128; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("RZalloc 1 KiB -> 2 KiB",
					  []() { TestAlloc([]() { return Memory::Zalloc(1024); },
									   [](void* data) { Memory::Free(data); },
									   [](void* data) { return Memory::RZalloc(data, 2048); },
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 256; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("Calloc 10x 1 KiB", []() { TestAlloc([]() { return Memory::Calloc(10, 1024); }, [](void* data) { Memory::Free(data); }); });
		Testing::Test("RCalloc 10x 1 KiB -> 10x 2 KiB", []() { TestAlloc([]() { return Memory::Calloc(10, 1024); }, [](void* data) { Memory::Free(data); }, [](void* data) { return Memory::RCalloc(data, 10, 2048); }); });
		Testing::Test("ZCalloc 10x 1 KiB",
					  []() { TestAlloc([]() { return Memory::ZCalloc(10, 1024); },
									   [](void* data) { Memory::Free(data); },
									   nullptr,
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 1280; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("RZCalloc 10x 1 KiB -> 10x 2 KiB",
					  []() { TestAlloc([]() { return Memory::ZCalloc(10, 1024); },
									   [](void* data) { Memory::Free(data); },
									   [](void* data) { return Memory::RZCalloc(data, 10, 2048); },
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 2560; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });

		Testing::Test("Malloc 1 MiB", []() { TestAlloc([]() { return Memory::Malloc(1024 << 10); }, [](void* data) { Memory::Free(data); }); });
		Testing::Test("RMalloc 1 MiB -> 2 MiB", []() { TestAlloc([]() { return Memory::Malloc(1024 << 10); }, [](void* data) { Memory::Free(data); }, [](void* data) { return Memory::RMalloc(data, 2048 << 10); }); });
		Testing::Test("Zalloc 1 MiB",
					  []() { TestAlloc([]() { return Memory::Zalloc(1024 << 10); },
									   [](void* data) { Memory::Free(data); },
									   nullptr,
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 131072; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("RZalloc 1 MiB -> 2 MiB",
					  []() { TestAlloc([]() { return Memory::Zalloc(1024 << 10); },
									   [](void* data) { Memory::Free(data); },
									   [](void* data) { return Memory::RZalloc(data, 2048 << 10); },
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 262144; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("Calloc 10x 1 MiB", []() { TestAlloc([]() { return Memory::Calloc(10, 1024 << 10); }, [](void* data) { Memory::Free(data); }); });
		Testing::Test("RCalloc 10x 1 MiB -> 10x 2 MiB", []() { TestAlloc([]() { return Memory::Calloc(10, 1024 << 10); }, [](void* data) { Memory::Free(data); }, [](void* data) { return Memory::RCalloc(data, 10, 2048 << 10); }); });
		Testing::Test("ZCalloc 10x 1 MiB",
					  []() { TestAlloc([]() { return Memory::ZCalloc(10, 1024 << 10); },
									   [](void* data) { Memory::Free(data); },
									   nullptr,
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 1310720; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("RZCalloc 10x 1 MiB -> 10x 2 MiB",
					  []() { TestAlloc([]() { return Memory::ZCalloc(10, 1024 << 10); },
									   [](void* data) { Memory::Free(data); },
									   [](void* data) { return Memory::RZCalloc(data, 10, 2048 << 10); },
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 2621440; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });


		Testing::Test("Malloc 1024x 1 KiB", []() { TestMultiAlloc<1024>([]() { return Memory::Malloc(1024); }, [](void* data) { Memory::Free(data); }); });
		Testing::Test("RMalloc 1024x 1 KiB -> 2 KiB", []() { TestMultiAlloc<1024>([]() { return Memory::Malloc(1024); }, [](void* data) { Memory::Free(data); }, [](void* data) { return Memory::RMalloc(data, 2048); }); });
		Testing::Test("Zalloc 1024x 1 KiB",
					  []() { TestMultiAlloc<1024>([]() { return Memory::Zalloc(1024); },
												  [](void* data) { Memory::Free(data); },
												  nullptr,
												  [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 128; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("RZalloc 1024x 1 KiB -> 2 KiB",
					  []() { TestMultiAlloc<1024>([]() { return Memory::Zalloc(1024); },
												  [](void* data) { Memory::Free(data); },
												  [](void* data) { return Memory::RZalloc(data, 2048); },
												  [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 256; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("Calloc 1024x 10x 1 KiB", []() { TestMultiAlloc<1024>([]() { return Memory::Calloc(10, 1024); }, [](void* data) { Memory::Free(data); }); });
		Testing::Test("RCalloc 1024x 10x 1 KiB -> 10x 2 KiB", []() { TestMultiAlloc<1024>([]() { return Memory::Calloc(10, 1024); }, [](void* data) { Memory::Free(data); }, [](void* data) { return Memory::RCalloc(data, 10, 2048); }); });
		Testing::Test("ZCalloc 1024x 10x 1 KiB",
					  []() { TestMultiAlloc<1024>([]() { return Memory::ZCalloc(10, 1024); },
												  [](void* data) { Memory::Free(data); },
												  nullptr,
												  [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 1280; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("RZCalloc 1024x 10x 1 KiB -> 10x 2 KiB",
					  []() { TestMultiAlloc<1024>([]() { return Memory::ZCalloc(10, 1024); },
												  [](void* data) { Memory::Free(data); },
												  [](void* data) { return Memory::RZCalloc(data, 10, 2048); },
												  [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 2560; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });

		Testing::Test("Malloc 1024x 1 MiB", []() { TestMultiAlloc<1024>([]() { return Memory::Malloc(1024 << 10); }, [](void* data) { Memory::Free(data); }); });
		Testing::Test("RMalloc 1024x 1 MiB -> 2 MiB", []() { TestMultiAlloc<1024>([]() { return Memory::Malloc(1024 << 10); }, [](void* data) { Memory::Free(data); }, [](void* data) { return Memory::RMalloc(data, 2048 << 10); }); });
		Testing::Test("Zalloc 1024x 1 MiB",
					  []() { TestMultiAlloc<1024>([]() { return Memory::Zalloc(1024 << 10); },
												  [](void* data) { Memory::Free(data); },
												  nullptr,
												  [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 131072; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("RZalloc 1024x 1 MiB -> 2 MiB",
					  []() { TestMultiAlloc<1024>([]() { return Memory::Zalloc(1024 << 10); },
												  [](void* data) { Memory::Free(data); },
												  [](void* data) { return Memory::RZalloc(data, 2048 << 10); },
												  [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 262144; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("Calloc 1024x 10x 1 MiB", []() { TestMultiAlloc<1024>([]() { return Memory::Calloc(10, 1024 << 10); }, [](void* data) { Memory::Free(data); }); });
		Testing::Test("RCalloc 1024x 10x 1 MiB -> 10x 2 MiB", []() { TestMultiAlloc<1024>([]() { return Memory::Calloc(10, 1024 << 10); }, [](void* data) { Memory::Free(data); }, [](void* data) { return Memory::RCalloc(data, 10, 2048 << 10); }); });
		Testing::Test("ZCalloc 1024x 10x 1 MiB",
					  []() { TestMultiAlloc<1024>([]() { return Memory::ZCalloc(10, 1024 << 10); },
												  [](void* data) { Memory::Free(data); },
												  nullptr,
												  [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 1310720; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("RZCalloc 1024x 10x 1 MiB -> 10x 2 MiB",
					  []() { TestMultiAlloc<1024>([]() { return Memory::ZCalloc(10, 1024 << 10); },
												  [](void* data) { Memory::Free(data); },
												  [](void* data) { return Memory::RZCalloc(data, 10, 2048 << 10); },
												  [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 2621440; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
	};

	auto advancedTests = []() {
		Testing::Test("AlignedMalloc 1 KiB", []() { TestAlloc([]() { return Memory::AlignedMalloc(256, 1024); }, [](void* data) { Memory::AlignedFree(data, 256); }); });
		Testing::Test("AlignedRMalloc 1 KiB -> 2 KiB", []() { TestAlloc([]() { return Memory::AlignedMalloc(256, 1024); }, [](void* data) { Memory::AlignedFree(data, 256); }, [](void* data) { return Memory::AlignedRMalloc(data, 2048, 256); }); });
		Testing::Test("AlignedZalloc 1 KiB",
					  []() { TestAlloc([]() { return Memory::AlignedZalloc(256, 1024); },
									   [](void* data) { Memory::AlignedFree(data, 256); },
									   nullptr,
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 128; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("AlignedRZalloc 1 KiB -> 2 KiB",
					  []() { TestAlloc([]() { return Memory::AlignedZalloc(256, 1024); },
									   [](void* data) { Memory::AlignedFree(data, 256); },
									   [](void* data) { return Memory::AlignedRZalloc(data, 2048, 256); },
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 256; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("AlignedCalloc 10x 1 KiB", []() { TestAlloc([]() { return Memory::AlignedCalloc(256, 10, 1024); }, [](void* data) { Memory::AlignedFree(data, 256); }); });
		Testing::Test("AlignedRCalloc 10x 1 KiB -> 10x 2 KiB", []() { TestAlloc([]() { return Memory::AlignedCalloc(256, 10, 1024); }, [](void* data) { Memory::AlignedFree(data, 256); }, [](void* data) { return Memory::AlignedRCalloc(data, 10, 2048, 256); }); });
		Testing::Test("AlignedZCalloc 10x 1 KiB",
					  []() { TestAlloc([]() { return Memory::AlignedZCalloc(256, 10, 1024); },
									   [](void* data) { Memory::AlignedFree(data, 256); },
									   nullptr,
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 1280; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("AlignedRZCalloc 10x 1 KiB -> 10x 2 KiB",
					  []() { TestAlloc([]() { return Memory::AlignedZCalloc(256, 10, 1024); },
									   [](void* data) { Memory::AlignedFree(data, 256); },
									   [](void* data) { return Memory::AlignedRZCalloc(data, 10, 2048, 256); },
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 2560; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });

		Testing::Test("AlignedMalloc 1 MiB", []() { TestAlloc([]() { return Memory::AlignedMalloc(256, 1024 << 10); }, [](void* data) { Memory::AlignedFree(data, 256); }); });
		Testing::Test("AlignedRMalloc 1 MiB -> 2 MiB", []() { TestAlloc([]() { return Memory::AlignedMalloc(256, 1024 << 10); }, [](void* data) { Memory::AlignedFree(data, 256); }, [](void* data) { return Memory::AlignedRMalloc(data, 2048 << 10, 256); }); });
		Testing::Test("AlignedZalloc 1 MiB",
					  []() { TestAlloc([]() { return Memory::AlignedZalloc(256, 1024 << 10); },
									   [](void* data) { Memory::AlignedFree(data, 256); },
									   nullptr,
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 131072; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("AlignedRZalloc 1 MiB -> 2 MiB",
					  []() { TestAlloc([]() { return Memory::AlignedZalloc(256, 1024 << 10); },
									   [](void* data) { Memory::AlignedFree(data, 256); },
									   [](void* data) { return Memory::AlignedRZalloc(data, 2048 << 10, 256); },
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 262144; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("AlignedCalloc 10x 1 MiB", []() { TestAlloc([]() { return Memory::AlignedCalloc(256, 10, 1024 << 10); }, [](void* data) { Memory::AlignedFree(data, 256); }); });
		Testing::Test("AlignedRCalloc 10x 1 MiB -> 10x 2 MiB", []() { TestAlloc([]() { return Memory::AlignedCalloc(256, 10, 1024 << 10); }, [](void* data) { Memory::AlignedFree(data, 256); }, [](void* data) { return Memory::AlignedRCalloc(data, 10, 2048 << 10, 256); }); });
		Testing::Test("AlignedZCalloc 10x 1 MiB",
					  []() { TestAlloc([]() { return Memory::AlignedZCalloc(256, 10, 1024 << 10); },
									   [](void* data) { Memory::AlignedFree(data, 256); },
									   nullptr,
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 1310720; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
		Testing::Test("AlignedRZCalloc 10x 1 MiB -> 10x 2 MiB",
					  []() { TestAlloc([]() { return Memory::AlignedZCalloc(256, 10, 1024 << 10); },
									   [](void* data) { Memory::AlignedFree(data, 256); },
									   [](void* data) { return Memory::AlignedRZCalloc(data, 10, 2048 << 10, 256); },
									   [](void* data) {
												 std::size_t* pData = reinterpret_cast<std::size_t*>(data);
												 for (std::size_t i = 0; i < 2621440; ++i)
												 {
													 if (pData[i] != 0)
														 Testing::Fail();
												 } }); });
	};

	Testing::PushGroup("Memory Basic w dbg");
	Allocator::SetDebugSettings({ .PushMessages = true });
	basicTests();
	Allocator::SetDebugSettings({ .PushMessages = false });
	Testing::PopGroup();
	Testing::PushGroup("Memory Basic w/o dbg");
	basicTests();
	Testing::PopGroup();

	Testing::PushGroup("Memory Advanced w dbg");
	Allocator::SetDebugSettings({ .PushMessages = true });
	advancedTests();
	Allocator::SetDebugSettings({ .PushMessages = false });
	Testing::PopGroup();
	Testing::PushGroup("Memory Advanced w/o dbg");
	advancedTests();
	Testing::PopGroup();

	double baselines[16];
	baselines[0] = Testing::TimedBasline([]() { TimedTestAlloc([]() { return std::malloc(1024); }, [](void* data) { std::free(data); }); });
	baselines[1] = Testing::TimedBasline([]() { TimedTestAlloc([]() { void* data = std::malloc(1024); std::memset(data, 0, 1024); return data; }, [](void* data) { std::free(data); }); });
	baselines[2] = Testing::TimedBasline([]() { TimedTestAlloc([]() { return std::calloc(10, 1024); }, [](void* data) { std::free(data); }); });
	baselines[3] = Testing::TimedBasline([]() { TimedTestAlloc([]() { void* data = std::calloc(10, 1024); std::memset(data, 0, 10240); return data; }, [](void* data) { std::free(data); }); });

	baselines[4] = Testing::TimedBasline([]() { TimedTestAlloc([]() { return std::malloc(1024); }, [](void* data) { std::free(data); }, [](void* data) { return std::realloc(data, 2048); }); });
	baselines[5] = Testing::TimedBasline([]() { TimedTestAlloc([]() { void* data = std::malloc(1024); std::memset(data, 0, 1024); return data; }, [](void* data) { std::free(data); }, [](void* data) { return std::realloc(data, 2048); }); });
	baselines[6] = Testing::TimedBasline([]() { TimedTestAlloc([]() { return std::calloc(10, 1024); }, [](void* data) { std::free(data); }, [](void* data) { return std::realloc(data, 10 * 2048); }); });
	baselines[7] = Testing::TimedBasline([]() { TimedTestAlloc([]() { void* data = std::calloc(10, 1024); std::memset(data, 0, 10240); return data; }, [](void* data) { std::free(data); }, [](void* data) { return std::realloc(data, 10 * 2048); }); });

	baselines[8]  = Testing::TimedBasline([]() { TimedTestAlloc([]() { return _aligned_malloc(1024, 256); }, [](void* data) { _aligned_free(data); }); });
	baselines[9]  = Testing::TimedBasline([]() { TimedTestAlloc([]() { void* data = _aligned_malloc(1024, 256); std::memset(data, 0, 1024); return data; }, [](void* data) { _aligned_free(data); }); });
	baselines[10] = Testing::TimedBasline([]() { TimedTestAlloc([]() { return _aligned_malloc(10 * 1024, 256); }, [](void* data) { _aligned_free(data); }); });
	baselines[11] = Testing::TimedBasline([]() { TimedTestAlloc([]() { void* data = _aligned_malloc(10 * 1024, 256); std::memset(data, 0, 10240); return data; }, [](void* data) { _aligned_free(data); }); });

	baselines[12] = Testing::TimedBasline([]() { TimedTestAlloc([]() { return _aligned_malloc(1024, 256); }, [](void* data) { _aligned_free(data); }, [](void* data) { return _aligned_realloc(data, 2048, 256); }); });
	baselines[13] = Testing::TimedBasline([]() { TimedTestAlloc([]() { void* data = _aligned_malloc(1024, 256); std::memset(data, 0, 1024); return data; }, [](void* data) { _aligned_free(data); }, [](void* data) { return _aligned_realloc(data, 2048, 256); }); });
	baselines[14] = Testing::TimedBasline([]() { TimedTestAlloc([]() { return _aligned_malloc(10 * 1024, 256); }, [](void* data) { _aligned_free(data); }, [](void* data) { return _aligned_recalloc(data, 10, 2048, 256); }); });
	baselines[15] = Testing::TimedBasline([]() { TimedTestAlloc([]() { void* data = _aligned_malloc(10 * 1024, 256); std::memset(data, 0, 10240); return data; }, [](void* data) { _aligned_free(data); }, [](void* data) { return _aligned_recalloc(data, 10, 2048, 256); }); });

	auto basicSpeedTests = [&baselines]() {
		Testing::TimedTest("Malloc Speed", baselines[0], []() { TimedTestAlloc([]() { return Memory::Malloc(1024); }, [](void* data) { Memory::Free(data); }); });
		Testing::TimedTest("Zalloc Speed", baselines[1], []() { TimedTestAlloc([]() { return Memory::Zalloc(1024); }, [](void* data) { Memory::Free(data); }); });
		Testing::TimedTest("Calloc Speed", baselines[2], []() { TimedTestAlloc([]() { return Memory::Calloc(10, 1024); }, [](void* data) { Memory::Free(data); }); });
		Testing::TimedTest("ZCalloc Speed", baselines[3], []() { TimedTestAlloc([]() { return Memory::ZCalloc(10, 1024); }, [](void* data) { Memory::Free(data); }); });

		Testing::TimedTest("RMalloc Speed", baselines[4], []() { TimedTestAlloc([]() { return Memory::Malloc(1024); }, [](void* data) { Memory::Free(data); }, [](void* data) { return Memory::RMalloc(data, 2048); }); });
		Testing::TimedTest("RZalloc Speed", baselines[5], []() { TimedTestAlloc([]() { return Memory::Zalloc(1024); }, [](void* data) { Memory::Free(data); }, [](void* data) { return Memory::RZalloc(data, 2048); }); });
		Testing::TimedTest("RCalloc Speed", baselines[6], []() { TimedTestAlloc([]() { return Memory::Calloc(10, 1024); }, [](void* data) { Memory::Free(data); }, [](void* data) { return Memory::RCalloc(data, 10, 2048); }); });
		Testing::TimedTest("RZCalloc Speed", baselines[7], []() { TimedTestAlloc([]() { return Memory::ZCalloc(10, 1024); }, [](void* data) { Memory::Free(data); }, [](void* data) { return Memory::RZCalloc(data, 10, 2048); }); });
	};

	auto advancedSpeedTests = [&baselines]() {
		Testing::TimedTest("AlignedMalloc Speed", baselines[8], []() { TimedTestAlloc([]() { return Memory::AlignedMalloc(256, 1024); }, [](void* data) { Memory::AlignedFree(data, 256); }); });
		Testing::TimedTest("AlignedZalloc Speed", baselines[9], []() { TimedTestAlloc([]() { return Memory::AlignedZalloc(256, 1024); }, [](void* data) { Memory::AlignedFree(data, 256); }); });
		Testing::TimedTest("AlignedCalloc Speed", baselines[10], []() { TimedTestAlloc([]() { return Memory::AlignedCalloc(256, 10, 1024); }, [](void* data) { Memory::AlignedFree(data, 256); }); });
		Testing::TimedTest("AlignedZCalloc Speed", baselines[11], []() { TimedTestAlloc([]() { return Memory::AlignedZCalloc(256, 10, 1024); }, [](void* data) { Memory::AlignedFree(data, 256); }); });

		Testing::TimedTest("AlignedRMalloc Speed", baselines[12], []() { TimedTestAlloc([]() { return Memory::AlignedMalloc(256, 1024); }, [](void* data) { Memory::AlignedFree(data, 256); }, [](void* data) { return Memory::AlignedRMalloc(data, 2048, 256); }); });
		Testing::TimedTest("AlignedRZalloc Speed", baselines[13], []() { TimedTestAlloc([]() { return Memory::AlignedZalloc(256, 1024); }, [](void* data) { Memory::AlignedFree(data, 256); }, [](void* data) { return Memory::AlignedRZalloc(data, 2048, 256); }); });
		Testing::TimedTest("AlignedRCalloc Speed", baselines[14], []() { TimedTestAlloc([]() { return Memory::AlignedCalloc(256, 10, 1024); }, [](void* data) { Memory::AlignedFree(data, 256); }, [](void* data) { return Memory::AlignedRCalloc(data, 10, 2048, 256); }); });
		Testing::TimedTest("AlignedRZCalloc Speed", baselines[15], []() { TimedTestAlloc([]() { return Memory::AlignedZCalloc(256, 10, 1024); }, [](void* data) { Memory::AlignedFree(data, 256); }, [](void* data) { return Memory::AlignedRZCalloc(data, 10, 2048, 256); }); });
	};

	Testing::PushGroup("Memory Basic Speed");
	basicSpeedTests();
	Testing::PopGroup();

	Testing::PushGroup("Memory Advanced Speed");
	advancedSpeedTests();
	Testing::PopGroup();
}