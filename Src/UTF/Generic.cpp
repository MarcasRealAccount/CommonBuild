#include "UTF/Generic.h"
#include "LUTs.h"

namespace UTF::Generic
{
	// TODO(MarcasRealAccount): Replace hard errors with encoding U+FFFD replacement character

	EError CalcReqSize8To16(const void* input, std::size_t inputSize, std::size_t& requiredSize)
	{
		requiredSize            = 0;
		const char8_t* inputBuf = reinterpret_cast<const char8_t*>(input);
		for (std::size_t i = 0; i < inputSize;)
		{
			char32_t     word = *reinterpret_cast<const char32_t*>(inputBuf);
			std::uint8_t size = LUTs::UTF8_6BitClass[(word >> 3) & 0x3F];
			switch (size)
			{
			case 1:
				requiredSize += 2;
				++inputBuf;
				++i;
				break;
			case 2:
				if ((word & 0x80C0) != 0x80C0)
					return EError::InvalidContinuation;
				requiredSize += 2;
				inputBuf     += 2;
				i            += 2;
				break;
			case 3:
				if ((word & 0x8080C0) != 0x8080C0)
					return EError::InvalidContinuation;
				requiredSize += 2;
				inputBuf     += 3;
				i            += 3;
				break;
			case 4:
				if ((word & 0x808080C0) != 0x808080C0)
					return EError::InvalidContinuation;
				requiredSize += 4;
				inputBuf     += 4;
				i            += 4;
				break;
			default:
				return EError::InvalidLeading;
			}
		}
		return EError::Success;
	}

	EError CalcReqSize8To32(const void* input, std::size_t inputSize, std::size_t& requiredSize)
	{
		requiredSize            = 0;
		const char8_t* inputBuf = reinterpret_cast<const char8_t*>(input);
		for (std::size_t i = 0; i < inputSize;)
		{
			char32_t     word = *reinterpret_cast<const char32_t*>(inputBuf);
			std::uint8_t size = LUTs::UTF8_6BitClass[(word >> 3) & 0x3F];
			switch (size)
			{
			case 1:
				requiredSize += 4;
				++inputBuf;
				++i;
				break;
			case 2:
				if ((word & 0x80C0) != 0x80C0)
					return EError::InvalidContinuation;
				requiredSize += 4;
				inputBuf     += 2;
				i            += 2;
				break;
			case 3:
				if ((word & 0x8080C0) != 0x8080C0)
					return EError::InvalidContinuation;
				requiredSize += 4;
				inputBuf     += 3;
				i            += 3;
				break;
			case 4:
				if ((word & 0x808080C0) != 0x808080C0)
					return EError::InvalidContinuation;
				requiredSize += 4;
				inputBuf     += 4;
				i            += 4;
				break;
			default:
				return EError::InvalidLeading;
			}
		}
		return EError::Success;
	}

	EError CalcReqSize16To8(const void* input, std::size_t inputSize, std::size_t& requiredSize)
	{
		requiredSize             = 0;
		const char16_t* inputBuf = reinterpret_cast<const char16_t*>(input);
		for (std::size_t i = 0; i < inputSize;)
		{
			char32_t     word = *reinterpret_cast<const char32_t*>(inputBuf);
			std::uint8_t size = LUTs::UTF16_6BitClass[(word >> 10) & 0x3F];
			switch (size)
			{
			case 1:
			{
				char32_t codepoint = word & 0xFFFF;
				if (codepoint < 0x80)
					++requiredSize;
				else if (codepoint < 0x800)
					requiredSize += 2;
				else
					requiredSize += 3;
				++inputBuf;
				i += 2;
				break;
			}
			case 2:
				if ((word & 0xDC00'D800) != 0xDC00'D800)
					return EError::InvalidContinuation;
				requiredSize += 4;
				inputBuf     += 2;
				i            += 4;
				break;
			default:
				return EError::InvalidLeading;
			}
		}
		return EError::Success;
	}

	EError CalcReqSize16To32(const void* input, std::size_t inputSize, std::size_t& requiredSize)
	{
		requiredSize             = 0;
		const char16_t* inputBuf = reinterpret_cast<const char16_t*>(input);
		for (std::size_t i = 0; i < inputSize;)
		{
			char32_t     word = *reinterpret_cast<const char32_t*>(inputBuf);
			std::uint8_t size = LUTs::UTF16_6BitClass[(word >> 10) & 0x3F];
			switch (size)
			{
			case 1:
			{
				requiredSize += 4;
				++inputBuf;
				i += 2;
				break;
			}
			case 2:
				if ((word & 0xDC00'D800) != 0xDC00'D800)
					return EError::InvalidContinuation;
				requiredSize += 4;
				inputBuf     += 2;
				i            += 4;
				break;
			default:
				return EError::InvalidLeading;
			}
		}
		return EError::Success;
	}

	EError CalcReqSize32To8(const void* input, std::size_t inputSize, std::size_t& requiredSize)
	{
		requiredSize             = 0;
		const char32_t* inputBuf = reinterpret_cast<const char32_t*>(input);
		for (std::size_t i = 0; i < inputSize;)
		{
			char32_t codepoint = *inputBuf;
			if (codepoint < 0x80)
				++requiredSize;
			else if (codepoint < 0x800)
				requiredSize += 2;
			else if (codepoint < 0x1'0000)
				requiredSize += 3;
			else if (codepoint < 0x11'0000)
				requiredSize += 4;
			else
				return EError::OOB;
			++inputBuf;
			i += 4;
		}
		return EError::Success;
	}

	EError CalcReqSize32To16(const void* input, std::size_t inputSize, std::size_t& requiredSize)
	{
		requiredSize             = 0;
		const char32_t* inputBuf = reinterpret_cast<const char32_t*>(input);
		for (std::size_t i = 0; i < inputSize;)
		{
			char32_t codepoint = *inputBuf;
			if (codepoint < 0x1'0000)
				requiredSize += 2;
			else if (codepoint < 0x11'0000)
				requiredSize += 4;
			else
				return EError::OOB;
			++inputBuf;
			i += 4;
		}
		return EError::Success;
	}

	EError ConvBlock8To16(const InputBlock& input, OutputBlock& output, std::size_t inputSize, std::size_t& outputSize)
	{
		outputSize               = 0;
		const char8_t* inputBuf  = reinterpret_cast<const char8_t*>(&input);
		char16_t*      outputBuf = reinterpret_cast<char16_t*>(&output);
		for (std::size_t i = 0; i < inputSize;)
		{
			char32_t     word      = *reinterpret_cast<const char32_t*>(inputBuf);
			std::uint8_t size      = LUTs::UTF8_6BitClass[(word >> 3) & 0x3F];
			char32_t     codepoint = 0;
			bool         errored   = false;
			switch (size)
			{
			case 1:
				codepoint = word & 0x7F;
				++inputBuf;
				++i;
				break;
			case 2:
				codepoint = (word & 0x1F) << 6 |
							((word >> 8) & 0x3F);
				inputBuf += 2;
				i        += 2;
				break;
			case 3:
				codepoint = (word & 0x0F) << 12 |
							((word >> 8) & 0x3F) << 6 |
							((word >> 16) & 0x3F);
				inputBuf += 3;
				i        += 3;
				break;
			case 4:
				codepoint = (word & 0x07) << 18 |
							((word >> 8) & 0x3F) << 12 |
							((word >> 16) & 0x3F) << 6 |
							((word >> 24) & 0x3F);
				inputBuf += 4;
				i        += 4;
				break;
			default:
				if (i >= 3)
					return EError::InvalidLeading;
				++inputBuf;
				++i;
				errored = true;
				break;
			}
			if (errored)
				continue;
			if (codepoint > 0xFFFF)
			{
				codepoint    = codepoint - 0x1'0000;
				outputBuf[0] = 0xD800 | ((codepoint >> 10) & 0x3FF);
				outputBuf[1] = 0xDC00 | (codepoint & 0x3FF);
				outputSize  += 4;
				outputBuf   += 2;
			}
			else
			{
				*outputBuf  = static_cast<char16_t>(codepoint);
				outputSize += 2;
				++outputBuf;
			}
		}
		return EError::Success;
	}

	EError ConvBlock8To32(const InputBlock& input, OutputBlock& output, std::size_t inputSize, std::size_t& outputSize)
	{
		outputSize               = 0;
		const char8_t* inputBuf  = reinterpret_cast<const char8_t*>(&input);
		char32_t*      outputBuf = reinterpret_cast<char32_t*>(&output);
		for (std::size_t i = 0; i < inputSize;)
		{
			char32_t     word = *reinterpret_cast<const char32_t*>(inputBuf);
			std::uint8_t size = LUTs::UTF8_6BitClass[(word >> 3) & 0x3F];
			switch (size)
			{
			case 1:
				*outputBuf = word & 0x7F;
				++inputBuf;
				++i;
				break;
			case 2:
				*outputBuf = (word & 0x1F) << 6 |
							 ((word >> 8) & 0x3F);
				inputBuf += 2;
				i        += 2;
				break;
			case 3:
				*outputBuf = (word & 0x0F) << 12 |
							 ((word >> 8) & 0x3F) << 6 |
							 ((word >> 16) & 0x3F);
				inputBuf += 3;
				i        += 3;
				break;
			case 4:
				*outputBuf = (word & 0x07) << 18 |
							 ((word >> 8) & 0x3F) << 12 |
							 ((word >> 16) & 0x3F) << 6 |
							 ((word >> 24) & 0x3F);
				inputBuf += 4;
				i        += 4;
				break;
			default:
				if (i >= 3)
					return EError::InvalidLeading;
				++inputBuf;
				++i;
				continue;
			}
			outputSize += 4;
			++outputBuf;
		}
		return EError::Success;
	}

	EError ConvBlock16To8(const InputBlock& input, OutputBlock& output, std::size_t inputSize, std::size_t& outputSize)
	{
		outputSize                = 0;
		const char16_t* inputBuf  = reinterpret_cast<const char16_t*>(&input);
		char8_t*        outputBuf = reinterpret_cast<char8_t*>(&output);
		for (std::size_t i = 0; i < inputSize;)
		{
			char32_t     word      = *reinterpret_cast<const char32_t*>(inputBuf);
			std::uint8_t size      = LUTs::UTF16_6BitClass[(word >> 10) & 0x3F];
			char32_t     codepoint = 0;
			switch (size)
			{
			case 1:
				codepoint = word & 0xFFFF;
				if (codepoint < 0x80)
				{
					*outputBuf = static_cast<char8_t>(codepoint & 0x7F);
					++outputBuf;
					++outputSize;
				}
				else if (codepoint < 0x800)
				{
					outputBuf[0] = 0xC0 | static_cast<char8_t>((codepoint >> 6) & 0x1F);
					outputBuf[1] = 0x80 | static_cast<char8_t>(codepoint & 0x3F);
					outputBuf   += 2;
					outputSize  += 2;
				}
				else
				{
					outputBuf[0] = 0xE0 | static_cast<char8_t>((codepoint >> 12) & 0x0F);
					outputBuf[1] = 0x80 | static_cast<char8_t>((codepoint >> 6) & 0x3F);
					outputBuf[2] = 0x80 | static_cast<char8_t>(codepoint & 0x3F);
					outputBuf   += 3;
					outputSize  += 3;
				}
				++inputBuf;
				i += 2;
				break;
			case 2:
				codepoint    = ((word & 0x3FF) << 10 | ((word >> 16) & 0x3FF)) + 0x1'0000;
				outputBuf[0] = 0xF0 | static_cast<char8_t>((codepoint >> 18) & 0x07);
				outputBuf[1] = 0x80 | static_cast<char8_t>((codepoint >> 12) & 0x3F);
				outputBuf[2] = 0x80 | static_cast<char8_t>((codepoint >> 6) & 0x3F);
				outputBuf[3] = 0x80 | static_cast<char8_t>(codepoint & 0x3F);
				outputBuf   += 4;
				outputSize  += 4;
				inputBuf    += 2;
				i           += 4;
				break;
			default:
				if (i >= 2)
					return EError::InvalidLeading;
				++i;
				++inputBuf;
			}
		}
		return EError::Success;
	}

	EError ConvBlock16To32(const InputBlock& input, OutputBlock& output, std::size_t inputSize, std::size_t& outputSize)
	{
		outputSize                = 0;
		const char16_t* inputBuf  = reinterpret_cast<const char16_t*>(&input);
		char32_t*       outputBuf = reinterpret_cast<char32_t*>(&output);
		for (std::size_t i = 0; i < inputSize;)
		{
			char32_t     word = *reinterpret_cast<const char32_t*>(inputBuf);
			std::uint8_t size = LUTs::UTF16_6BitClass[(word >> 10) & 0x3F];
			switch (size)
			{
			case 1:
				*outputBuf = word & 0xFFFF;
				++outputBuf;
				outputSize += 4;
				++inputBuf;
				i += 2;
				break;
			case 2:
				*outputBuf = ((word & 0x3FF) << 10 | ((word >> 16) & 0x3FF)) + 0x1'0000;
				++outputBuf;
				outputSize += 4;
				inputBuf   += 2;
				i          += 4;
				break;
			default:
				if (i >= 2)
					return EError::InvalidLeading;
				++i;
				++inputBuf;
			}
		}
		return EError::Success;
	}

	EError ConvBlock32To8(const InputBlock& input, OutputBlock& output, std::size_t inputSize, std::size_t& outputSize)
	{
		outputSize                = 0;
		const char32_t* inputBuf  = reinterpret_cast<const char32_t*>(&input);
		char8_t*        outputBuf = reinterpret_cast<char8_t*>(&output);
		for (std::size_t i = 0; i < inputSize;)
		{
			char32_t codepoint = *inputBuf;
			if (codepoint < 0x80)
			{
				*outputBuf = static_cast<char8_t>(codepoint & 0x7F);
				++outputBuf;
				++outputSize;
			}
			else if (codepoint < 0x800)
			{
				outputBuf[0] = 0xC0 | static_cast<char8_t>((codepoint >> 6) & 0x1F);
				outputBuf[1] = 0x80 | static_cast<char8_t>(codepoint & 0x3F);
				outputBuf   += 2;
				outputSize  += 2;
			}
			else if (codepoint < 0x1'0000)
			{
				outputBuf[0] = 0xE0 | static_cast<char8_t>((codepoint >> 12) & 0x0F);
				outputBuf[1] = 0x80 | static_cast<char8_t>((codepoint >> 6) & 0x3F);
				outputBuf[2] = 0x80 | static_cast<char8_t>(codepoint & 0x3F);
				outputBuf   += 3;
				outputSize  += 3;
			}
			else if (codepoint < 0x11'0000)
			{
				outputBuf[0] = 0xF0 | static_cast<char8_t>((codepoint >> 18) & 0x07);
				outputBuf[1] = 0x80 | static_cast<char8_t>((codepoint >> 12) & 0x3F);
				outputBuf[2] = 0x80 | static_cast<char8_t>((codepoint >> 6) & 0x3F);
				outputBuf[3] = 0x80 | static_cast<char8_t>(codepoint & 0x3F);
				outputBuf   += 4;
				outputSize  += 4;
			}
			else
			{
				return EError::OOB;
			}
			++inputBuf;
			i += 4;
		}
		return EError::Success;
	}

	EError ConvBlock32To16(const InputBlock& input, OutputBlock& output, std::size_t inputSize, std::size_t& outputSize)
	{
		outputSize                = 0;
		const char32_t* inputBuf  = reinterpret_cast<const char32_t*>(&input);
		char16_t*       outputBuf = reinterpret_cast<char16_t*>(&output);
		for (std::size_t i = 0; i < inputSize;)
		{
			char32_t codepoint = *inputBuf;
			if (codepoint < 0x1'0000)
			{
				*outputBuf = static_cast<char16_t>(codepoint & 0xFFFF);
				++outputBuf;
				outputSize += 2;
			}
			else if (codepoint < 0x11'0000)
			{
				codepoint   -= 0x1'0000;
				outputBuf[0] = 0xD800 | static_cast<char16_t>((codepoint >> 10) & 0x3FF);
				outputBuf[1] = 0xDC00 | static_cast<char16_t>(codepoint & 0x3FF);
				outputBuf   += 2;
				outputSize  += 4;
			}
			else
			{
				return EError::OOB;
			}
			++inputBuf;
			i += 4;
		}
		return EError::Success;
	}
} // namespace UTF::Generic
