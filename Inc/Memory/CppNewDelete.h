#include "Memory.h"

#include <new>

[[nodiscard]] void* operator new(std::size_t count)
{
	void* ptr = Memory::Malloc(count);
	if (!ptr)
		throw std::bad_alloc {};
	return ptr;
}

[[nodiscard]] void* operator new[](std::size_t count)
{
	void* ptr = Memory::Malloc(count);
	if (!ptr)
		throw std::bad_alloc {};
	return ptr;
}

[[nodiscard]] void* operator new(std::size_t count, std::align_val_t al)
{
	void* ptr = Memory::AlignedMalloc(static_cast<std::size_t>(al), count);
	if (!ptr)
		throw std::bad_alloc {};
	return ptr;
}

[[nodiscard]] void* operator new[](std::size_t count, std::align_val_t al)
{
	void* ptr = Memory::AlignedMalloc(static_cast<std::size_t>(al), count);
	if (!ptr)
		throw std::bad_alloc {};
	return ptr;
}

[[nodiscard]] void* operator new(std::size_t count, const std::nothrow_t&) noexcept
{
	return Memory::Malloc(count);
}

[[nodiscard]] void* operator new[](std::size_t count, const std::nothrow_t&) noexcept
{
	return Memory::Malloc(count);
}

[[nodiscard]] void* operator new(std::size_t count, std::align_val_t al, const std::nothrow_t&) noexcept
{
	return Memory::AlignedMalloc(static_cast<std::size_t>(al), count);
}

[[nodiscard]] void* operator new[](std::size_t count, std::align_val_t al, const std::nothrow_t&) noexcept
{
	return Memory::AlignedMalloc(static_cast<std::size_t>(al), count);
}

void operator delete(void* ptr) noexcept
{
	Memory::Free(ptr);
}

void operator delete[](void* ptr) noexcept
{
	Memory::Free(ptr);
}

void operator delete(void* ptr, std::align_val_t al) noexcept
{
	Memory::AlignedFree(ptr, static_cast<std::size_t>(al));
}

void operator delete[](void* ptr, std::align_val_t al) noexcept
{
	Memory::AlignedFree(ptr, static_cast<std::size_t>(al));
}

void operator delete(void* ptr, [[maybe_unused]] std::size_t sz) noexcept
{
	Memory::Free(ptr);
}

void operator delete[](void* ptr, [[maybe_unused]] std::size_t sz) noexcept
{
	Memory::Free(ptr);
}

void operator delete(void* ptr, [[maybe_unused]] std::size_t sz, std::align_val_t al) noexcept
{
	Memory::AlignedFree(ptr, static_cast<std::size_t>(al));
}

void operator delete[](void* ptr, [[maybe_unused]] std::size_t sz, std::align_val_t al) noexcept
{
	Memory::AlignedFree(ptr, static_cast<std::size_t>(al));
}