#include "Concurrency/Mutex.h"
#include "Build.h"

#if BUILD_IS_SYSTEM_WINDOWS
	#include <Windows.h>
#endif

namespace Concurrency
{
	static std::uint64_t GetThreadID() noexcept
	{
#if BUILD_IS_SYSTEM_WINDOWS
		return GetCurrentThreadId();
#else
		return 1;
#endif
	}

	void Mutex::Lock() noexcept
	{
		bool val = false;
		while (val = false, !m_Value.compare_exchange_weak(val, true))
			m_Value.wait(val);
	}

	bool Mutex::TryLock() noexcept
	{
		bool val = false;
		return m_Value.compare_exchange_weak(val, true);
	}

	void Mutex::Unlock() noexcept
	{
		m_Value.store(false);
		m_Value.notify_all();
	}

	void RecursiveMutex::Lock() noexcept
	{
		std::uint64_t tid = GetThreadID();
		std::uint64_t to  = tid << 32 | 1;
		std::uint64_t val = 0ULL;
		while (val = 0ULL, !m_Value.compare_exchange_weak(val, to))
		{
			if ((val >> 32) == tid)
			{
				m_Value.fetch_add(1ULL);
				break;
			}
			m_Value.wait(val);
		}
	}

	bool RecursiveMutex::TryLock() noexcept
	{
		std::uint64_t tid = GetThreadID();
		std::uint64_t to  = tid << 32 | 1;
		std::uint64_t val = 0ULL;
		bool          res = m_Value.compare_exchange_weak(val, to);
		if (!res && (val >> 32) == tid)
		{
			m_Value.fetch_add(1ULL);
			return true;
		}
		return res;
	}

	void RecursiveMutex::Unlock() noexcept
	{
		m_Value.fetch_sub(1ULL);
		m_Value.notify_all();
	}

	void SharedMutex::Lock() noexcept
	{
		std::uint64_t tid = GetThreadID();
		std::uint64_t to  = tid << 32 | 0x8000'0001;
		std::uint64_t val = 0ULL;
		while (val = 0ULL, !m_Value.compare_exchange_weak(val, to))
			m_Value.wait(val);
	}

	void SharedMutex::LockShared() noexcept
	{
		std::uint64_t tid = GetThreadID();
		std::uint64_t val = m_Value.load();
		while (val & 0x8000'0000 && (val >> 32 != tid))
		{
			m_Value.wait(val);
			val = m_Value.load();
		}
		m_Value.fetch_add(1ULL);
	}

	bool SharedMutex::TryLock() noexcept
	{
		std::uint64_t tid = GetThreadID();
		std::uint64_t to  = tid << 32 | 0x8000'0001;
		std::uint64_t val = 0ULL;
		return m_Value.compare_exchange_weak(val, to);
	}

	bool SharedMutex::TryLockShared() noexcept
	{
		std::uint64_t val = m_Value.load();
		if (val & 0x8000'0000 && (val >> 32 != tid))
			return false;
		m_Value.fetch_add(1ULL);
		return true;
	}

	void SharedMutex::Unlock() noexcept
	{
		if ((m_Value.fetch_sub(1ULL) & 0x7FFF'FFFF) == 1)
			m_Value.store(0ULL);
		m_Value.notify_all();
	}

	void SharedMutex::UnlockShared() noexcept
	{
		if ((m_Value.fetch_sub(1ULL) & 0x7FFF'FFFF) == 1)
			m_Value.store(0ULL);
		m_Value.notify_all();
	}

	void RecursiveSharedMutex::Lock() noexcept
	{
		std::uint64_t tid = GetThreadID();
		std::uint64_t to  = tid << 32 | 0x8000'0001;
		std::uint64_t val = 0ULL;
		while (val = 0ULL, !m_Value.compare_exchange_weak(val, to))
		{
			if ((val >> 32) == tid)
			{
				m_Value.fetch_add(1ULL);
				break;
			}
			m_Value.wait(val);
		}
	}

	void RecursiveSharedMutex::LockShared() noexcept
	{
		std::uint64_t tid = GetThreadID();
		std::uint64_t val = m_Value.load();
		while (val & 0x8000'0000 && (val >> 32 != tid))
		{
			m_Value.wait(val);
			val = m_Value.load();
		}
		m_Value.fetch_add(1ULL);
	}

	bool RecursiveSharedMutex::TryLock() noexcept
	{
		std::uint64_t tid = GetThreadID();
		std::uint64_t to  = tid << 32 | 0x8000'0001;
		std::uint64_t val = 0ULL;
		bool          res = m_Value.compare_exchange_weak(val, to);
		if (!res && (val >> 32) == tid)
		{
			m_Value.fetch_add(1ULL);
			return true;
		}
		return res;
	}

	bool RecursiveSharedMutex::TryLockShared() noexcept
	{
		std::uint64_t tid = GetThreadID();
		std::uint64_t val = m_Value.load();
		if (val & 0x8000'0000 && (val >> 32 != tid))
			return false;
		m_Value.fetch_add(1ULL);
		return true;
	}

	void RecursiveSharedMutex::Unlock() noexcept
	{
		if ((m_Value.fetch_sub(1ULL) & 0x7FFF'FFFF) == 1)
			m_Value.store(0ULL);
		m_Value.notify_all();
	}

	void RecursiveSharedMutex::UnlockShared() noexcept
	{
		if ((m_Value.fetch_sub(1ULL) & 0x7FFF'FFFF) == 1)
			m_Value.store(0ULL);
		m_Value.notify_all();
	}
} // namespace Concurrency
