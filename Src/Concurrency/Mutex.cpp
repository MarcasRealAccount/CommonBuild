#include "Concurrency/Mutex.h"
#include "Build.h"

#ifdef BUILD_IS_SYSTEM_WINDOWS
	#include <Windows.h>
#endif

namespace Concurrency
{
	static std::uint64_t GetThreadID()
	{
#if BUILD_IS_SYSTEM_WINDOWS
		return GetCurrentThreadId();
#else
		return 0;
#endif
	}

	void Mutex::Lock()
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

	void Mutex::Unlock()
	{
		m_Value.store(false);
		m_Value.notify_all();
	}

	void RecursiveMutex::Lock()
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

	void RecursiveMutex::Unlock()
	{
		m_Value.fetch_sub(1ULL);
		m_Value.notify_all();
	}

	void SharedMutex::Lock()
	{
		std::uint64_t tid = GetThreadID();
		std::uint64_t to  = tid << 32 | 0x8000'0001;
		std::uint64_t val = 0ULL;
		while (val = 0ULL, !m_Value.compare_exchange_weak(val, to))
			m_Value.wait(val);
	}

	void SharedMutex::LockShared()
	{
		std::uint64_t tid = GetThreadID();
		std::uint64_t val = m_Value.load();
		while (val & 0x8000'0000 && !m_Value.compare_exchange_weak(val, val + 1))
		{
			m_Value.wait(val);
			val = m_Value.load();
		}
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
		std::uint64_t tid = GetThreadID();
		std::uint64_t val = m_Value.load();
		return !(val & 0x8000'0000) && m_Value.compare_exchange_weak(val, val + 1);
	}

	void SharedMutex::Unlock()
	{
		if ((m_Value.fetch_sub(1ULL) & 0x7FFF'FFFF) == 1)
			m_Value.store(0ULL);
		m_Value.notify_all();
	}

	void SharedMutex::UnlockShared()
	{
		m_Value.fetch_sub(1ULL);
		m_Value.notify_all();
	}

	void RecursiveSharedMutex::Lock()
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

	void RecursiveSharedMutex::LockShared()
	{
		std::uint64_t tid = GetThreadID();
		std::uint64_t val = m_Value.load();
		if ((val >> 32) == tid)
		{
			m_Value.fetch_add(1ULL);
			return;
		}
		while (val & 0x8000'0000 && !m_Value.compare_exchange_weak(val, val + 1))
		{
			m_Value.wait(val);
			val = m_Value.load();
		}
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
		if ((val >> 32) == tid)
		{
			m_Value.fetch_add(1ULL);
			return true;
		}
		return !(val & 0x8000'0000) && m_Value.compare_exchange_weak(val, val + 1);
	}

	void RecursiveSharedMutex::Unlock()
	{
		if ((m_Value.fetch_sub(1ULL) & 0x7FFF'FFFF) == 1)
			m_Value.store(0ULL);
		m_Value.notify_all();
	}

	void RecursiveSharedMutex::UnlockShared()
	{
		m_Value.fetch_sub(1ULL);
		m_Value.notify_all();
	}
} // namespace Concurrency