#pragma once

#include "UTFLuts.h"

#include <cstddef>
#include <cstdint>

#include <string>
#include <string_view>
#include <utility>

namespace UTF
{
	inline std::pair<std::size_t, std::size_t> NumFastIterations(std::size_t byteCount)
	{
		std::size_t fast = (byteCount - 4) / 64;
		return { fast, byteCount - fast * 64 };
	}

	inline std::size_t UTF8ToUTF32RequiredSizeBlock(const std::uint8_t* utf8, std::size_t* utf8Read)
	{
		std::size_t i = 0, j = 0;
		for (; i < 64; ++i)
		{
			std::uint32_t word = *reinterpret_cast<const std::uint32_t*>(utf8 + i);
			switch (LUTs::UTF86BitClass[(word >> 3) & 0x3F])
			{
			case 1: ++j; break;
			case 2: ++j; break;
			case 3: ++j; break;
			case 4: ++j; break;
			}
		}
		if (utf8Read)
			*utf8Read = 64;
		return j;
	}

	inline std::size_t UTF8ToUTF16RequiredSizeBlock(const std::uint8_t* utf8, std::size_t* utf8Read)
	{
		std::size_t i = 0, j = 0;
		for (; i < 64; ++i)
		{
			std::uint32_t word = *reinterpret_cast<const std::uint32_t*>(utf8 + i);
			switch (LUTs::UTF86BitClass[(word >> 3) & 0x3F])
			{
			case 1: ++j; break;
			case 2: ++j; break;
			case 3: ++j; break;
			case 4: j += 2; break;
			}
		}
		if (utf8Read)
			*utf8Read = 64;
		return j;
	}

	inline std::size_t UTF16ToUTF32RequiredSizeBlock(const std::uint16_t* utf16, std::size_t* utf16Read)
	{
		std::size_t i = 0, j = 0;
		for (; i < 32; ++i)
		{
			std::uint32_t word = *reinterpret_cast<const std::uint32_t*>(utf16 + i);
			switch (LUTs::UTF166BitClass[(word >> 10) & 0x3F])
			{
			case 1: ++j; break;
			case 2: ++j; break;
			}
		}
		if (utf16Read)
			*utf16Read = 32;
		return j;
	}

	inline std::size_t UTF16ToUTF8RequiredSizeBlock(const std::uint16_t* utf16, std::size_t* utf16Read)
	{
		std::size_t i = 0, j = 0;
		for (; i < 32; ++i)
		{
			std::uint32_t word = *reinterpret_cast<const std::uint32_t*>(utf16 + i);
			std::uint32_t codepoint;
			switch (LUTs::UTF166BitClass[(word >> 10) & 0x3F])
			{
			case 1:
				codepoint = word & 0xFFFF;
				if (codepoint < 0x80)
					++j;
				else if (codepoint < 0x800)
					j += 2;
				else if (codepoint < 0x10'000)
					j += 3;
				break;
			case 2:
				j += 4;
				break;
			}
		}
		if (utf16Read)
			*utf16Read = 32;
		return j;
	}

	inline std::size_t UTF32ToUTF16RequiredSizeBlock(const std::uint32_t* codepoints, std::size_t* codepointsRead)
	{
		std::size_t i = 0, j = 0;
		for (; i < 16; ++i)
		{
			std::uint32_t codepoint = codepoints[i];
			if (codepoint < 0xFFFF)
				++j;
			else
				j += 2;
		}
		if (codepointsRead)
			*codepointsRead = 16;
		return j;
	}

	inline std::size_t UTF32ToUTF8RequiredSizeBlock(const std::uint32_t* codepoints, std::size_t* codepointsRead)
	{
		std::size_t i = 0, j = 0;
		for (; i < 16; ++i)
		{
			std::uint32_t codepoint = codepoints[i];
			if (codepoint < 0x80)
				++j;
			else if (codepoint < 0x800)
				j += 2;
			else if (codepoint < 0x10000)
				j += 3;
			else
				j += 4;
		}
		if (codepointsRead)
			*codepointsRead = 16;
		return j;
	}

	inline void UTF8ToUTF32Block(std::uint32_t* codepoints, const std::uint8_t* utf8, std::size_t* utf8Read, std::size_t* codepointsWritten)
	{
		std::size_t i = 0, j = 0;
		for (; i < 64; ++i)
		{
			std::uint32_t word = *reinterpret_cast<const std::uint32_t*>(utf8 + i);
			switch (LUTs::UTF86BitClass[(word >> 3) & 0x3F])
			{
			case 1: codepoints[j++] = word & 0x7F; break;
			case 2: codepoints[j++] = word & 0x1F << 6 | (word >> 8) & 0x3F; break;
			case 3: codepoints[j++] = word & 0x0F << 12 | ((word >> 8) & 0x3F) << 6 | (word >> 16) & 0x3F; break;
			case 4: codepoints[j++] = word & 0x07 << 18 | ((word >> 8) & 0x3F) << 12 | ((word >> 16) & 0x3F) << 6 | (word >> 24) & 0x3F; break;
			}
		}
		if (utf8Read)
			*utf8Read = 64;
		if (codepointsWritten)
			*codepointsWritten = j;
	}

	inline void UTF8ToUTF16Block(std::uint16_t* utf16, const std::uint8_t* utf8, std::size_t* utf8Read, std::size_t* utf16Written)
	{
		std::size_t i = 0, j = 0;
		for (; i < 64; ++i)
		{
			std::uint32_t word = *reinterpret_cast<const std::uint32_t*>(utf8 + i);
			std::uint32_t codepoint;
			switch (LUTs::UTF86BitClass[(word >> 3) & 0x3F])
			{
			case 1: utf16[j++] = word & 0x7F; break;
			case 2: utf16[j++] = word & 0x1F << 6 | (word >> 8) & 0x3F; break;
			case 3: utf16[j++] = word & 0x0F << 12 | ((word >> 8) & 0x3F) << 6 | (word >> 16) & 0x3F; break;
			case 4:
				codepoint  = (word & 0x03 << 18 | ((word >> 8) & 0x3F) << 12 | ((word >> 16) & 0x3F) << 6 | (word >> 24) & 0x3F);
				utf16[j++] = static_cast<std::uint16_t>(0xD800 + (codepoint >> 10));
				utf16[j++] = static_cast<std::uint16_t>(0xDC00 + (codepoint & 0x3FF));
				break;
			}
		}
		if (utf8Read)
			*utf8Read = 64;
		if (utf16Written)
			*utf16Written = j;
	}

	inline void UTF16ToUTF32Block(std::uint32_t* codepoints, const std::uint16_t* utf16, std::size_t* utf16Read, std::size_t* codepointsWritten)
	{
		std::size_t i = 0, j = 0;
		for (; i < 32; ++i)
		{
			std::uint32_t word = *reinterpret_cast<const std::uint32_t*>(utf16 + i);
			switch (LUTs::UTF166BitClass[(word >> 10) & 0x3F])
			{
			case 1:
				codepoints[j++] = word & 0xFFFF;
				break;
			case 2:
				codepoints[j++] = 0x10'000 + (word & 0x3FF | ((word >> 16) & 0x3FF) << 10);
				break;
			}
		}
		if (utf16Read)
			*utf16Read = 32;
		if (codepointsWritten)
			*codepointsWritten = j;
	}

	inline void UTF16ToUTF8Block(std::uint8_t* utf8, const std::uint16_t* utf16, std::size_t* utf16Read, std::size_t* utf8Written)
	{
		std::size_t i = 0, j = 0;
		for (; i < 32; ++i)
		{
			std::uint32_t word = *reinterpret_cast<const std::uint32_t*>(utf16 + i);
			std::uint32_t codepoint;
			switch (LUTs::UTF166BitClass[(word >> 10) & 0x3F])
			{
			case 1:
				codepoint = word & 0xFFFF;
				if (codepoint < 0x80)
				{
					utf8[j++] = codepoint & 0x7F;
				}
				else if (codepoint < 0x800)
				{
					utf8[j++] = 0b110'00000 | (codepoint >> 6) & 0x1F;
					utf8[j++] = 0b10'000000 | codepoint & 0x3F;
				}
				else if (codepoint < 0x10'000)
				{
					utf8[j++] = 0b1110'0000 | (codepoint >> 12) & 0xF;
					utf8[j++] = 0b10'000000 | (codepoint >> 6) & 0x3F;
					utf8[j++] = 0b10'000000 | codepoint & 0x3F;
				}
				break;
			case 2:
				codepoint = 0x10'000 + (word & 0x3FF | ((word >> 16) & 0x3FF) << 10);
				utf8[j++] = 0b11110'000 | (codepoint >> 18) & 0x7;
				utf8[j++] = 0b10'000000 | (codepoint >> 12) & 0x3F;
				utf8[j++] = 0b10'000000 | (codepoint >> 6) & 0x3F;
				utf8[j++] = 0b10'000000 | codepoint & 0x3F;
				break;
			}
		}
		if (utf16Read)
			*utf16Read = 32;
		if (utf8Written)
			*utf8Written = j;
	}

	inline void UTF32ToUTF16Block(std::uint16_t* utf16, const std::uint32_t* codepoints, std::size_t* codepointsRead, std::size_t* utf16Written)
	{
		std::size_t i = 0, j = 0;
		for (; i < 16; ++i)
		{
			std::uint32_t codepoint = codepoints[i];
			if (codepoint < 0xFFFF)
			{
				utf16[j++] = static_cast<std::uint16_t>(codepoint);
			}
			else
			{
				auto c     = codepoint - 0x10'000;
				utf16[j++] = static_cast<std::uint16_t>(0xD800 + (codepoint >> 10));
				utf16[j++] = static_cast<std::uint16_t>(0xDC00 + (codepoint & 0x3FF));
			}
		}
		if (codepointsRead)
			*codepointsRead = 16;
		if (utf16Written)
			*utf16Written = j;
	}

	inline void UTF32ToUTF8Block(std::uint8_t* utf8, const std::uint32_t* codepoints, std::size_t* codepointsRead, std::size_t* utf8Written)
	{
		std::size_t i = 0, j = 0;
		for (; i < 16; ++i)
		{
			std::uint32_t codepoint = codepoints[i];
			if (codepoint < 0x80)
			{
				utf8[j++] = static_cast<std::uint8_t>(codepoint);
			}
			else if (codepoint < 0x800)
			{
				utf8[j++] = static_cast<std::uint8_t>((codepoint >> 6) & 0x1F);
				utf8[j++] = static_cast<std::uint8_t>(codepoint & 0x3F);
			}
			else if (codepoint < 0x10000)
			{
				utf8[j++] = static_cast<std::uint8_t>((codepoint >> 12) & 0x0F);
				utf8[j++] = static_cast<std::uint8_t>((codepoint >> 6) & 0x3F);
				utf8[j++] = static_cast<std::uint8_t>(codepoint & 0x3F);
			}
			else
			{
				utf8[j++] = static_cast<std::uint8_t>((codepoint >> 18) & 0x07);
				utf8[j++] = static_cast<std::uint8_t>((codepoint >> 12) & 0x3F);
				utf8[j++] = static_cast<std::uint8_t>((codepoint >> 6) & 0x3F);
				utf8[j++] = static_cast<std::uint8_t>(codepoint & 0x3F);
			}
		}
		if (codepointsRead)
			*codepointsRead = 16;
		if (utf8Written)
			*utf8Written = j;
	}

	inline std::size_t CharToUTF32RequiredSize(std::string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char8_t));

		std::size_t result = 0;

		const std::uint8_t* pStr = reinterpret_cast<const std::uint8_t*>(str.data());
		std::size_t         read = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			result += UTF8ToUTF32RequiredSizeBlock(pStr + i * 64, &read);
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 64, slowIterations);
			result += UTF8ToUTF32RequiredSizeBlock(reinterpret_cast<const std::uint8_t*>(block), &read);
		}

		return result;
	}

	inline std::size_t CharToUTF16RequiredSize(std::string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char8_t));

		std::size_t result = 0;

		const std::uint8_t* pStr = reinterpret_cast<const std::uint8_t*>(str.data());
		std::size_t         read = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			result += UTF8ToUTF16RequiredSizeBlock(pStr + i * 64, &read);
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 64, slowIterations);
			result += UTF8ToUTF16RequiredSizeBlock(reinterpret_cast<const std::uint8_t*>(block), &read);
		}

		return result;
	}

	inline std::size_t CharToWideRequiredSize(std::string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char8_t));

		std::size_t result = 0;

		const std::uint8_t* pStr = reinterpret_cast<const std::uint8_t*>(str.data());
		std::size_t         read = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			result += UTF8ToUTF16RequiredSizeBlock(pStr + i * 64, &read);
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 64, slowIterations);
			result += UTF8ToUTF16RequiredSizeBlock(reinterpret_cast<const std::uint8_t*>(block), &read);
		}

		return result;
	}

	inline std::size_t WideToUTF32RequiredSize(std::wstring_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char16_t));

		std::size_t result = 0;

		const std::uint16_t* pStr = reinterpret_cast<const std::uint16_t*>(str.data());
		std::size_t          read = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			result += UTF16ToUTF32RequiredSizeBlock(pStr + i * 32, &read);
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 32, slowIterations);
			result += UTF16ToUTF32RequiredSizeBlock(reinterpret_cast<const std::uint16_t*>(block), &read);
		}

		return result;
	}

	inline std::size_t WideToUTF8RequiredSize(std::wstring_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char16_t));

		std::size_t result = 0;

		const std::uint16_t* pStr = reinterpret_cast<const std::uint16_t*>(str.data());
		std::size_t          read = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			result += UTF16ToUTF8RequiredSizeBlock(pStr + i * 32, &read);
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 32, slowIterations);
			result += UTF16ToUTF8RequiredSizeBlock(reinterpret_cast<const std::uint16_t*>(block), &read);
		}

		return result;
	}

	inline std::size_t WideToCharRequiredSize(std::wstring_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char16_t));

		std::size_t result = 0;

		const std::uint16_t* pStr = reinterpret_cast<const std::uint16_t*>(str.data());
		std::size_t          read = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			result += UTF16ToUTF8RequiredSizeBlock(pStr + i * 32, &read);
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 32, slowIterations);
			result += UTF16ToUTF8RequiredSizeBlock(reinterpret_cast<const std::uint16_t*>(block), &read);
		}

		return result;
	}

	inline std::size_t UTF32ToWideRequiredSize(std::u32string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char32_t));

		std::size_t result = 0;

		const std::uint32_t* pStr = reinterpret_cast<const std::uint32_t*>(str.data());
		std::size_t          read = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			result += UTF32ToUTF16RequiredSizeBlock(pStr + i * 16, &read);
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 16, slowIterations);
			result += UTF32ToUTF16RequiredSizeBlock(reinterpret_cast<const std::uint32_t*>(block), &read);
		}

		return result;
	}

	inline std::size_t UTF32ToCharRequiredSize(std::u32string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char32_t));

		std::size_t result = 0;

		const std::uint32_t* pStr = reinterpret_cast<const std::uint32_t*>(str.data());
		std::size_t          read = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			result += UTF32ToUTF8RequiredSizeBlock(pStr + i * 16, &read);
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 16, slowIterations);
			result += UTF32ToUTF8RequiredSizeBlock(reinterpret_cast<const std::uint32_t*>(block), &read);
		}

		return result;
	}

	inline std::size_t UTF8ToUTF32RequiredSize(std::u8string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char8_t));

		std::size_t result = 0;

		const std::uint8_t* pStr = reinterpret_cast<const std::uint8_t*>(str.data());
		std::size_t         read = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			result += UTF8ToUTF32RequiredSizeBlock(pStr + i * 64, &read);
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 64, slowIterations);
			result += UTF8ToUTF32RequiredSizeBlock(reinterpret_cast<const std::uint8_t*>(block), &read);
		}

		return result;
	}

	inline std::size_t UTF8ToUTF16RequiredSize(std::u8string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char8_t));

		std::size_t result = 0;

		const std::uint8_t* pStr = reinterpret_cast<const std::uint8_t*>(str.data());
		std::size_t         read = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			result += UTF8ToUTF16RequiredSizeBlock(pStr + i * 64, &read);
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 64, slowIterations);
			result += UTF8ToUTF16RequiredSizeBlock(reinterpret_cast<const std::uint8_t*>(block), &read);
		}

		return result;
	}

	inline std::size_t UTF16ToUTF32RequiredSize(std::u16string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char16_t));

		std::size_t result = 0;

		const std::uint16_t* pStr = reinterpret_cast<const std::uint16_t*>(str.data());
		std::size_t          read = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			result += UTF16ToUTF32RequiredSizeBlock(pStr + i * 32, &read);
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 32, slowIterations);
			result += UTF16ToUTF32RequiredSizeBlock(reinterpret_cast<const std::uint16_t*>(block), &read);
		}

		return result;
	}

	inline std::size_t UTF16ToUTF8RequiredSize(std::u16string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char16_t));

		std::size_t result = 0;

		const std::uint16_t* pStr = reinterpret_cast<const std::uint16_t*>(str.data());
		std::size_t          read = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			result += UTF16ToUTF8RequiredSizeBlock(pStr + i * 32, &read);
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 32, slowIterations);
			result += UTF16ToUTF8RequiredSizeBlock(reinterpret_cast<const std::uint16_t*>(block), &read);
		}

		return result;
	}

	inline std::size_t UTF32ToUTF16RequiredSize(std::u32string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char32_t));

		std::size_t result = 0;

		const std::uint32_t* pStr = reinterpret_cast<const std::uint32_t*>(str.data());
		std::size_t          read = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			result += UTF32ToUTF16RequiredSizeBlock(pStr + i * 16, &read);
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 16, slowIterations);
			result += UTF32ToUTF16RequiredSizeBlock(reinterpret_cast<const std::uint32_t*>(block), &read);
		}

		return result;
	}

	inline std::size_t UTF32ToUTF8RequiredSize(std::u32string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char32_t));

		std::size_t result = 0;

		const std::uint32_t* pStr = reinterpret_cast<const std::uint32_t*>(str.data());
		std::size_t          read = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			result += UTF32ToUTF8RequiredSizeBlock(pStr + i * 16, &read);
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 16, slowIterations);
			result += UTF32ToUTF8RequiredSizeBlock(reinterpret_cast<const std::uint32_t*>(block), &read);
		}

		return result;
	}

	inline std::u16string CharToUTF16(std::string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char8_t));
		std::size_t    outputSize             = CharToUTF16RequiredSize(str);
		std::u16string output(outputSize + 32, u8'\0');

		std::uint16_t*      pOutput = reinterpret_cast<std::uint16_t*>(output.data());
		const std::uint8_t* pStr    = reinterpret_cast<const std::uint8_t*>(str.data());
		std::size_t         offset  = 0;
		std::size_t         read = 0, written = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			UTF8ToUTF16Block(pOutput + offset, pStr + i * 64, &read, &written);
			offset += written;
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 32, slowIterations);
			UTF8ToUTF16Block(pOutput + offset, reinterpret_cast<const std::uint8_t*>(block), &read, &written);
			offset += written;
		}

		output.resize(offset);

		return output;
	}

	inline std::wstring CharToWide(std::string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char8_t));
		std::size_t  outputSize               = CharToWideRequiredSize(str);
		std::wstring output(outputSize + 32, u8'\0');

		std::uint16_t*      pOutput = reinterpret_cast<std::uint16_t*>(output.data());
		const std::uint8_t* pStr    = reinterpret_cast<const std::uint8_t*>(str.data());
		std::size_t         offset  = 0;
		std::size_t         read = 0, written = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			UTF8ToUTF16Block(pOutput + offset, pStr + i * 64, &read, &written);
			offset += written;
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 32, slowIterations);
			UTF8ToUTF16Block(pOutput + offset, reinterpret_cast<const std::uint8_t*>(block), &read, &written);
			offset += written;
		}

		output.resize(offset);

		return output;
	}

	inline std::u32string CharToUTF32(std::string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char8_t));
		std::size_t    outputSize             = CharToUTF32RequiredSize(str);
		std::u32string output(outputSize + 16, U'\0');

		std::uint32_t*      pOutput = reinterpret_cast<std::uint32_t*>(output.data());
		const std::uint8_t* pStr    = reinterpret_cast<const std::uint8_t*>(str.data());
		std::size_t         offset  = 0;
		std::size_t         read = 0, written = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			UTF8ToUTF32Block(pOutput + offset, pStr + i * 64, &read, &written);
			offset += written;
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 64, slowIterations);
			UTF8ToUTF32Block(pOutput + offset, reinterpret_cast<const std::uint8_t*>(block), &read, &written);
			offset += written;
		}

		output.resize(offset);

		return output;
	}

	inline std::u32string WideToUTF32(std::wstring_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char16_t));
		std::size_t    outputSize             = WideToUTF32RequiredSize(str);
		std::u32string output(outputSize + 16, U'\0');

		std::uint32_t*       pOutput = reinterpret_cast<std::uint32_t*>(output.data());
		const std::uint16_t* pStr    = reinterpret_cast<const std::uint16_t*>(str.data());
		std::size_t          offset  = 0;
		std::size_t          read = 0, written = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			UTF16ToUTF32Block(pOutput + offset, pStr + i * 32, &read, &written);
			offset += written;
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 32, slowIterations);
			UTF16ToUTF32Block(pOutput + offset, reinterpret_cast<const std::uint16_t*>(block), &read, &written);
			offset += written;
		}

		output.resize(offset);

		return output;
	}

	inline std::u8string WideToUTF8(std::wstring_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char16_t));
		std::size_t   outputSize              = WideToUTF8RequiredSize(str);
		std::u8string output(outputSize + 64, u8'\0');

		std::uint8_t*        pOutput = reinterpret_cast<std::uint8_t*>(output.data());
		const std::uint16_t* pStr    = reinterpret_cast<const std::uint16_t*>(str.data());
		std::size_t          offset  = 0;
		std::size_t          read = 0, written = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			UTF16ToUTF8Block(pOutput + offset, pStr + i * 32, &read, &written);
			offset += written;
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 32, slowIterations);
			UTF16ToUTF8Block(pOutput + offset, reinterpret_cast<const std::uint16_t*>(block), &read, &written);
			offset += written;
		}

		output.resize(offset);

		return output;
	}

	inline std::string WideToChar(std::wstring_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char16_t));
		std::size_t outputSize                = WideToCharRequiredSize(str);
		std::string output(outputSize + 64, u8'\0');

		std::uint8_t*        pOutput = reinterpret_cast<std::uint8_t*>(output.data());
		const std::uint16_t* pStr    = reinterpret_cast<const std::uint16_t*>(str.data());
		std::size_t          offset  = 0;
		std::size_t          read = 0, written = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			UTF16ToUTF8Block(pOutput + offset, pStr + i * 32, &read, &written);
			offset += written;
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 32, slowIterations);
			UTF16ToUTF8Block(pOutput + offset, reinterpret_cast<const std::uint16_t*>(block), &read, &written);
			offset += written;
		}

		output.resize(offset);

		return output;
	}

	inline std::wstring UTF32ToWide(std::u32string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char32_t));
		std::size_t  outputSize               = UTF32ToUTF16RequiredSize(str);
		std::wstring output(outputSize + 32, U'\0');

		std::uint16_t*       pOutput = reinterpret_cast<std::uint16_t*>(output.data());
		const std::uint32_t* pStr    = reinterpret_cast<const std::uint32_t*>(str.data());
		std::size_t          offset  = 0;
		std::size_t          read = 0, written = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			UTF32ToUTF16Block(pOutput + offset, pStr + i * 16, &read, &written);
			offset += written;
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 16, slowIterations);
			UTF32ToUTF16Block(pOutput + offset, reinterpret_cast<const std::uint32_t*>(block), &read, &written);
			offset += written;
		}

		output.resize(offset);

		return output;
	}

	inline std::string UTF32ToChar(std::u32string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char16_t));
		std::size_t outputSize                = UTF32ToUTF8RequiredSize(str);
		std::string output(outputSize + 64, u8'\0');

		std::uint8_t*        pOutput = reinterpret_cast<std::uint8_t*>(output.data());
		const std::uint32_t* pStr    = reinterpret_cast<const std::uint32_t*>(str.data());
		std::size_t          offset  = 0;
		std::size_t          read = 0, written = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			UTF32ToUTF8Block(pOutput + offset, pStr + i * 16, &read, &written);
			offset += written;
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 16, slowIterations);
			UTF32ToUTF8Block(pOutput + offset, reinterpret_cast<const std::uint32_t*>(block), &read, &written);
			offset += written;
		}

		output.resize(offset);

		return output;
	}

	inline std::u32string UTF8ToUTF32(std::u8string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char8_t));
		std::size_t    outputSize             = UTF8ToUTF32RequiredSize(str);
		std::u32string output(outputSize + 16, U'\0');

		std::uint32_t*      pOutput = reinterpret_cast<std::uint32_t*>(output.data());
		const std::uint8_t* pStr    = reinterpret_cast<const std::uint8_t*>(str.data());
		std::size_t         offset  = 0;
		std::size_t         read = 0, written = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			UTF8ToUTF32Block(pOutput + offset, pStr + i * 64, &read, &written);
			offset += written;
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 64, slowIterations);
			UTF8ToUTF32Block(pOutput + offset, reinterpret_cast<const std::uint8_t*>(block), &read, &written);
			offset += written;
		}

		output.resize(offset);

		return output;
	}

	inline std::u16string UTF8ToUTF16(std::u8string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char8_t));
		std::size_t    outputSize             = UTF8ToUTF16RequiredSize(str);
		std::u16string output(outputSize + 32, u8'\0');

		std::uint16_t*      pOutput = reinterpret_cast<std::uint16_t*>(output.data());
		const std::uint8_t* pStr    = reinterpret_cast<const std::uint8_t*>(str.data());
		std::size_t         offset  = 0;
		std::size_t         read = 0, written = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			UTF8ToUTF16Block(pOutput + offset, pStr + i * 64, &read, &written);
			offset += written;
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 32, slowIterations);
			UTF8ToUTF16Block(pOutput + offset, reinterpret_cast<const std::uint8_t*>(block), &read, &written);
			offset += written;
		}

		output.resize(offset);

		return output;
	}

	inline std::u32string UTF16ToUTF32(std::u16string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char16_t));
		std::size_t    outputSize             = UTF16ToUTF32RequiredSize(str);
		std::u32string output(outputSize + 16, U'\0');

		std::uint32_t*       pOutput = reinterpret_cast<std::uint32_t*>(output.data());
		const std::uint16_t* pStr    = reinterpret_cast<const std::uint16_t*>(str.data());
		std::size_t          offset  = 0;
		std::size_t          read = 0, written = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			UTF16ToUTF32Block(pOutput + offset, pStr + i * 32, &read, &written);
			offset += written;
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 32, slowIterations);
			UTF16ToUTF32Block(pOutput + offset, reinterpret_cast<const std::uint16_t*>(block), &read, &written);
			offset += written;
		}

		output.resize(offset);

		return output;
	}

	inline std::u8string UTF16ToUTF8(std::u16string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char16_t));
		std::size_t   outputSize              = UTF16ToUTF8RequiredSize(str);
		std::u8string output(outputSize + 64, u8'\0');

		std::uint8_t*        pOutput = reinterpret_cast<std::uint8_t*>(output.data());
		const std::uint16_t* pStr    = reinterpret_cast<const std::uint16_t*>(str.data());
		std::size_t          offset  = 0;
		std::size_t          read = 0, written = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			UTF16ToUTF8Block(pOutput + offset, pStr + i * 32, &read, &written);
			offset += written;
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 32, slowIterations);
			UTF16ToUTF8Block(pOutput + offset, reinterpret_cast<const std::uint16_t*>(block), &read, &written);
			offset += written;
		}

		output.resize(offset);

		return output;
	}

	inline std::u16string UTF32ToUTF16(std::u32string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char32_t));
		std::size_t    outputSize             = UTF32ToUTF16RequiredSize(str);
		std::u16string output(outputSize + 32, U'\0');

		std::uint16_t*       pOutput = reinterpret_cast<std::uint16_t*>(output.data());
		const std::uint32_t* pStr    = reinterpret_cast<const std::uint32_t*>(str.data());
		std::size_t          offset  = 0;
		std::size_t          read = 0, written = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			UTF32ToUTF16Block(pOutput + offset, pStr + i * 16, &read, &written);
			offset += written;
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 16, slowIterations);
			UTF32ToUTF16Block(pOutput + offset, reinterpret_cast<const std::uint32_t*>(block), &read, &written);
			offset += written;
		}

		output.resize(offset);

		return output;
	}

	inline std::u8string UTF32ToUTF8(std::u32string_view str)
	{
		auto [fastIterations, slowIterations] = NumFastIterations(str.size() * sizeof(char16_t));
		std::size_t   outputSize              = UTF32ToUTF8RequiredSize(str);
		std::u8string output(outputSize + 64, u8'\0');

		std::uint8_t*        pOutput = reinterpret_cast<std::uint8_t*>(output.data());
		const std::uint32_t* pStr    = reinterpret_cast<const std::uint32_t*>(str.data());
		std::size_t          offset  = 0;
		std::size_t          read = 0, written = 0;
		for (std::size_t i = 0; i < fastIterations; ++i)
		{
			UTF32ToUTF8Block(pOutput + offset, pStr + i * 16, &read, &written);
			offset += written;
		}

		if (slowIterations)
		{
			alignas(64) std::uint8_t block[64] {};
			std::memcpy(block, pStr + fastIterations * 16, slowIterations);
			UTF32ToUTF8Block(pOutput + offset, reinterpret_cast<const std::uint32_t*>(block), &read, &written);
			offset += written;
		}

		output.resize(offset);

		return output;
	}
} // namespace UTF