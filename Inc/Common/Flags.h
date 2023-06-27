#pragma once

#include <cstdint>

#include <concepts>

namespace Common
{
	namespace Details
	{
		template <class T>
		concept HasBitwiseAnd = requires(T t) { { t & t } -> std::convertible_to<T>; };
		template <class T>
		concept HasBitwiseOr = requires(T t) { { t | t } -> std::convertible_to<T>; };
		template <class T>
		concept HasBitwiseXor = requires(T t) { { t ^ t } -> std::convertible_to<T>; };
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
		concept Flaggable = std::is_pod_v<T> && sizeof(T) <= 16 &&
							HasBitwiseOr<T> && HasBitwiseXor<T> && HasBitwiseAnd<T> && HasBitwiseNot<T> &&
							HasLeftShift<T> && HasRightShift<T> &&
							HasEquals<T> && HasLessThan<T> && HasGreaterThan<T>;
	} // namespace Details

	template <Details::Flaggable T = std::uint32_t>
	struct Flags
	{
	public:
		using Type = T;

	public:
		T Value;

	public:
		constexpr Flags() noexcept
			: Value(T { 0 }) {}

		constexpr Flags(T value) noexcept
			: Value(value) {}

		constexpr Flags(const Flags& copy) noexcept
			: Value(copy.Value) {}

		constexpr Flags& operator=(T value) noexcept
		{
			Value = value;
			return *this;
		}

		constexpr Flags& operator=(const Flags& copy) noexcept
		{
			Value = copy.Value;
			return *this;
		}

		constexpr bool HasFlag(Flags flags) const
		{
			return (*this & flags) == flags;
		}

		constexpr operator bool()
		{
			return Value != T { 0 };
		}

		constexpr operator T()
		{
			return Value;
		}

#define UNARY_OP(Op)                                 \
	constexpr friend Flags operator##Op(Flags flags) \
	{                                                \
		return { Op##flags.Value };                  \
	}
#define BINARY_FRIEND_OP(Op, RhsType, RhsName, Rhs)                 \
	constexpr friend Flags operator##Op(Flags lhs, RhsType RhsName) \
	{                                                               \
		return { lhs.Value Op Rhs };                                \
	}
#define BINARY_OP(Op, RhsType, RhsName, Rhs)          \
	BINARY_FRIEND_OP(Op, RhsType, RhsName, Rhs)       \
	constexpr Flags& operator##Op##=(RhsType RhsName) \
	{                                                 \
		Value = Value Op Rhs;                         \
		return *this;                                 \
	}

		UNARY_OP(~);
		BINARY_OP(|, Flags, flags, flags.Value);
		BINARY_OP(&, Flags, flags, flags.Value);
		BINARY_OP(^, Flags, flags, flags.Value);
		BINARY_OP(>>, int, count, count);
		BINARY_OP(<<, int, count, count);
		BINARY_FRIEND_OP(==, Flags, flags, flags.Value);
		BINARY_FRIEND_OP(!=, Flags, flags, flags.Value);
		BINARY_FRIEND_OP(<, Flags, flags, flags.Value);
		BINARY_FRIEND_OP(>, Flags, flags, flags.Value);
		BINARY_FRIEND_OP(<=, Flags, flags, flags.Value);
		BINARY_FRIEND_OP(>=, Flags, flags, flags.Value);

#undef UNARY_OP
#undef BINARY_FRIEND_OP
#undef BINARY_OP
	};
} // namespace Common