#pragma once

#include <cstdint>

#include <concepts>
#include <type_traits>
#include <utility>

namespace CommonBuild
{
	namespace Detail
	{
		template <class T>
		concept HasBitwiseAnd = requires(T t) { { t & t } -> std::convertible_to<T>; };
		template <class T>
		concept HasBitwiseOr = requires(T t) { { t | t } -> std::convertible_to<T>; };
		template <class T>
		concept HasBitwiseNot = requires(T t) { { ~t } -> std::convertible_to<T>; };
		template <class T>
		concept HasLeftShift = requires(T t, std::size_t n) { { t << n } -> std::convertible_to<T>; };
		template <class T>
		concept HasRightShift = requires(T t, std::size_t n) { { t >> n } -> std::convertible_to<T>; };
		template <class T>
		concept HasEquals = requires(T t) { { t == t } -> std::convertible_to<bool>; };
		template <class T>
		concept HasLessThan = requires(T t) { { t < t } -> std::convertible_to<bool>; };
		template <class T>
		concept HasGreaterThan = requires(T t) { { t > t } -> std::convertible_to<bool>; };

		template <class T>
		concept Flaggable = std::is_pod_v<T> && sizeof(T) < 16 &&
							HasBitwiseOr<T> && HasBitwiseAnd<T> && HasBitwiseNot<T> && HasLeftShift<T> && HasRightShift<T> &&
							HasEquals<T> && HasLessThan<T> && HasGreaterThan<T>;
	} // namespace Detail

	template <Detail::Flaggable T = std::uint32_t>
	struct Flags
	{
	public:
		T Value;

	public:
		constexpr Flags() noexcept
			: Value(T { 0 }) {}

		constexpr Flags(std::convertible_to<T> auto&& value) noexcept
			: Value(static_cast<T>(value)) {}

		constexpr Flags(const Flags& copy) noexcept
			: Value(copy.Value) {}

		constexpr Flags& operator=(std::convertible_to<T> auto&& value) noexcept
		{
			Value = static_cast<T>(value);
			return *this;
		}

		constexpr Flags& operator=(const Flags& copy) noexcept
		{
			Value = copy.Value;
			return *this;
		}

		constexpr bool hasFlag(Flags flags) const { return (Value & flags.Value) != T { 0 }; }

		// clang-format off
		constexpr operator bool() { return Value != T { 0 }; }
		constexpr operator T() { return Value; }

		constexpr Flags& operator|=(Flags flags) { Value = Value | flags.Value; return *this; }
		constexpr Flags& operator&=(Flags flags) { Value = Value & flags.Value; return *this; }
		constexpr Flags& operator^=(Flags flags) { Value = Value ^ flags.Value; return *this; }
		constexpr Flags& operator>>=(std::size_t count) { Value = Value >> count; return *this; }
		constexpr Flags& operator<<=(std::size_t count) { Value = Value << count; return *this; }
		constexpr friend Flags operator|(const Flags& lhs, Flags rhs) { return { lhs.Value | rhs.Value }; }
		constexpr friend Flags operator&(const Flags& lhs, Flags rhs) { return { lhs.Value & rhs.Value }; }
		constexpr friend Flags operator^(const Flags& lhs, Flags rhs) { return { lhs.Value ^ rhs.Value }; }
		constexpr friend Flags operator~(const Flags& flags) { return { ~flags.Value }; }
		constexpr friend Flags operator>>=(const Flags& flags, std::size_t count) { return { flags.Value >> count }; }
		constexpr friend Flags operator<<=(const Flags& flags, std::size_t count) { return { flags.Value << count }; }

		constexpr friend bool operator==(const Flags& lhs, Flags rhs) { return lhs.Value == rhs.Value; }
		constexpr friend bool operator<(const Flags& lhs, Flags rhs) { return lhs.Value < rhs.Value; }
		constexpr friend bool operator>(const Flags& lhs, Flags rhs) { return lhs.Value > rhs.Value; }
		constexpr friend bool operator!=(const Flags& lhs, Flags rhs) { return !(lhs == rhs); }
		constexpr friend bool operator>=(const Flags& lhs, Flags rhs) { return !(lhs < rhs); }
		constexpr friend bool operator<=(const Flags& lhs, Flags rhs) { return !(lhs > rhs); }

		// clang-format on
	};
} // namespace CommonBuild