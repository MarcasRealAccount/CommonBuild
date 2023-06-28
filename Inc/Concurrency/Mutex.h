#pragma once

#include <atomic>
#include <concepts>
#include <tuple>

namespace Concurrency
{
	template <class T>
	concept MutexC = requires {
		{
			T::Lock()
		} -> std::same_as<void>;
		{
			T::TryLock()
		} -> std::same_as<bool>;
		{
			T::Unlock()
		} -> std::same_as<void>;
	};
	template <class T>
	concept SharedMutexC = MutexC<T> && requires {
		{
			T::LockShared()
		} -> std::same_as<void>;
		{
			T::TryLockShared()
		} -> std::same_as<bool>;
		{
			T::UnlockShared()
		} -> std::same_as<void>;
	};

	struct Mutex
	{
	public:
		constexpr Mutex() noexcept
			: m_Value(false) {}

		~Mutex() {}

		Mutex(const Mutex&)            = delete;
		Mutex& operator=(const Mutex&) = delete;

		void Lock() noexcept;
		bool TryLock() noexcept;
		void Unlock() noexcept;

	private:
		std::atomic_bool m_Value;
	};

	struct RecursiveMutex
	{
	public:
		constexpr RecursiveMutex() noexcept
			: m_Value(0ULL) {}

		~RecursiveMutex() {}

		RecursiveMutex(const RecursiveMutex&)            = delete;
		RecursiveMutex& operator=(const RecursiveMutex&) = delete;

		void Lock() noexcept;
		bool TryLock() noexcept;
		void Unlock() noexcept;

	private:
		std::atomic_uint64_t m_Value;
	};

	struct SharedMutex
	{
	public:
		constexpr SharedMutex() noexcept
			: m_Value(0ULL) {}

		~SharedMutex() {}

		SharedMutex(const SharedMutex&)            = delete;
		SharedMutex& operator=(const SharedMutex&) = delete;

		void Lock() noexcept;
		void LockShared() noexcept;
		bool TryLock() noexcept;
		bool TryLockShared() noexcept;
		void Unlock() noexcept;
		void UnlockShared() noexcept;

	private:
		std::atomic_uint64_t m_Value;
	};

	struct RecursiveSharedMutex
	{
	public:
		constexpr RecursiveSharedMutex() noexcept
			: m_Value(0ULL) {}

		~RecursiveSharedMutex() {}

		RecursiveSharedMutex(const RecursiveSharedMutex&)            = delete;
		RecursiveSharedMutex& operator=(const RecursiveSharedMutex&) = delete;

		void Lock() noexcept;
		void LockShared() noexcept;
		bool TryLock() noexcept;
		bool TryLockShared() noexcept;
		void Unlock() noexcept;
		void UnlockShared() noexcept;

	private:
		std::atomic_uint64_t m_Value;
	};

	template <MutexC... Mutexes>
	void Lock(Mutexes&... mutexes) noexcept
	{
		(mutexes.Lock(), ...);
	}

	template <SharedMutexC... Mutexes>
	void LockShared(Mutexes&... mutexes) noexcept
	{
		(mutexes.LockShared(), ...);
	}

	template <MutexC... Mutexes>
	std::size_t TryLock(Mutexes&... mutexes) noexcept
	{
		std::size_t count = 0;
		(count += mutexes.TryLock() ? 1 : 0, ...);
		return count;
	}

	template <SharedMutexC... Mutexes>
	std::size_t TryLockShared(Mutexes&... mutexes) noexcept
	{
		std::size_t count = 0;
		(count += mutexes.TryLockShared() ? 1 : 0, ...);
		return count;
	}

	template <MutexC... Mutexes>
	void Unlock(Mutexes&... mutexes) noexcept
	{
		(mutexes.Unlock(), ...);
	}

	template <SharedMutexC... Mutexes>
	void UnlockShared(Mutexes&... mutexes) noexcept
	{
		(mutexes.UnlockShared(), ...);
	}

	template <MutexC... Mutexes>
	struct ScopedLock
	{
	public:
		explicit ScopedLock(Mutexes&... mutexes) noexcept
			: m_Mutexes(std::tie(mutexes...))
		{
			Lock(mutexes...);
		}

		~ScopedLock() noexcept
		{
			Unlock(std::make_index_sequence<sizeof...(Mutexes)> {});
		}

		ScopedLock(const ScopedLock&)            = delete;
		ScopedLock& operator=(const ScopedLock&) = delete;

	private:
		template <std::size_t... Indices>
		void Unlock(const std::index_sequence<Indices...>&) noexcept
		{
			(std::get<Indices>(m_Mutexes).Unlock(), ...);
		}

	private:
		std::tuple<Mutexes&...> m_Mutexes;
	};

	template <SharedMutexC... Mutexes>
	struct ScopedSharedLock
	{
	public:
		explicit ScopedSharedLock(Mutexes&... mutexes) noexcept
			: m_Mutexes(std::tie(mutexes...))
		{
			LockShared(mutexes...);
		}

		~ScopedSharedLock() noexcept
		{
			Unlock(std::make_index_sequence<sizeof...(Mutexes)>);
		}

		ScopedSharedLock(const ScopedSharedLock&)            = delete;
		ScopedSharedLock& operator=(const ScopedSharedLock&) = delete;

	private:
		template <std::size_t... Indices>
		void Unlock(const std::index_sequence<Indices...>&) noexcept
		{
			(std::get<Indices>(m_Mutexes).UnlockShared(), ...);
		}

	private:
		std::tuple<Mutexes&...> m_Mutexes;
	};
} // namespace Concurrency