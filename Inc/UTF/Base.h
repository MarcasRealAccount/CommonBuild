#pragma once

#include "Build.h"

#include <cstdint>

namespace UTF
{
	enum class EEncoding : std::uint8_t
	{
		UTF8 = 0,
		UTF16,
		UTF32
	};

	static constexpr std::uint8_t c_EncodingCount = 3;

	enum class EError
	{
		Success,
		MissingImpl,
		OOB,
		InvalidLeading,
		InvalidContinuation
	};

	struct alignas(64) InputBlock
	{
		std::uint8_t Bytes[128];
	};

	struct alignas(64) OutputBlock
	{
		std::uint8_t Bytes[256];
	};

	namespace Details
	{
		template <EEncoding Encoding = EEncoding::UTF8>
		struct CharType
		{
			using Type = char8_t;
		};

		template <>
		struct CharType<EEncoding::UTF16>
		{
			using Type = char16_t;
		};

		template <>
		struct CharType<EEncoding::UTF32>
		{
			using Type = char32_t;
		};

		template <EEncoding Encoding>
		using CharTypeT = typename CharType<Encoding>::Type;

		template <class C = char>
		struct EncodingType
		{
			static constexpr EEncoding Value = EEncoding::UTF8;
		};

		template <>
		struct EncodingType<wchar_t>
		{
#if BUILD_IS_SYSTEM_WINDOWS
			static constexpr EEncoding Value = EEncoding::UTF16;
#else
			static constexpr EEncoding Value = EEncoding::UTF32;
#endif
		};

		template <>
		struct EncodingType<char8_t>
		{
			static constexpr EEncoding Value = EEncoding::UTF8;
		};

		template <>
		struct EncodingType<char16_t>
		{
			static constexpr EEncoding Value = EEncoding::UTF16;
		};

		template <>
		struct EncodingType<char32_t>
		{
			static constexpr EEncoding Value = EEncoding::UTF32;
		};

		template <class C>
		static constexpr EEncoding EncodingTypeV = EncodingType<C>::Value;
	} // namespace Details
} // namespace UTF