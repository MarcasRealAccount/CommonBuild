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
	};

	Testing::PushGroup("Memory Basic w dbg");
	Allocator::SetDebugSettings({ .PushMessages = true });
	basicTests();
	Allocator::SetDebugSettings({ .PushMessages = false });
	Testing::PopGroup();
	Testing::PushGroup("Memory Basic w/o dbg");
	basicTests();
	Testing::PopGroup();
}