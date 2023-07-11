#pragma once

#include "Base.h"
#include "Generic.h"
#include "Memory/Memory.h"
#include "SIMD.h"

#include <concepts>
#include <string>
#include <string_view>

namespace UTF
{
	namespace Details
	{
		template <class T, class C>
		concept String = std::same_as<T, std::basic_string<C>>;
		template <class T, class C>
		concept StringView = std::same_as<T, std::basic_string_view<C>>;

		constexpr void CalcIters(std::uintptr_t address, std::size_t size, std::size_t alignment, std::size_t& firstBytes, std::size_t& lastBytes)
		{
			std::uintptr_t alignedAddress  = Memory::AlignCeil(address, alignment);
			std::uintptr_t lastAlignedAddr = Memory::AlignFloor(address + size, alignment);

			if ((alignedAddress - address) >= size)
			{
				firstBytes = size;
				lastBytes  = 0;
				return;
			}
			if ((lastAlignedAddr - address) >= size)
			{
				firstBytes = alignedAddress - address;
				lastBytes  = 0;
				return;
			}

			firstBytes = alignedAddress - address;
			lastBytes  = size - firstBytes - (lastAlignedAddr - alignedAddress);
		}
	} // namespace Details

	using CalcReqSizeImplF = EError (*)(const void* input, std::size_t inputSize, std::size_t& requiredSize);
	using ConvBlockImplF   = EError (*)(const InputBlock& input, OutputBlock& output, std::size_t inputSize, std::size_t& outputSize);

	enum class EImpl : std::uint8_t
	{
		Generic = 0,
		SIMD    = 1,
		Fastest
	};

	static constexpr std::uint8_t c_ImplCount = 2;
	extern CalcReqSizeImplF       s_CalcReqSizeImpls[c_EncodingCount][c_EncodingCount][c_ImplCount];
	extern ConvBlockImplF         s_ConvBlockImpls[c_EncodingCount][c_EncodingCount][c_ImplCount];

	EImpl GetFastestImpl();

	template <EEncoding From, EEncoding To>
	requires(From != To)
	EError CalcReqSize(const void* input, std::size_t inputSize, std::size_t& requiredSize, EImpl impl = EImpl::Fastest)
	{
		if (impl == EImpl::Fastest)
			impl = GetFastestImpl();

		auto callback = s_CalcReqSizeImpls[static_cast<std::uint8_t>(From)][static_cast<std::uint8_t>(To)][static_cast<std::uint8_t>(impl)];
		if (!callback)
			return EError::MissingImpl;
		return callback(input, inputSize, requiredSize);
	}

	template <EEncoding From, EEncoding To>
	requires(From != To)
	EError ConvBlock(const InputBlock& input, OutputBlock& output, std::size_t inputSize, std::size_t& outputSize, EImpl impl = EImpl::Fastest)
	{
		if (impl == EImpl::Fastest)
			impl = GetFastestImpl();

		auto callback = s_ConvBlockImpls[static_cast<std::uint8_t>(From)][static_cast<std::uint8_t>(To)][static_cast<std::uint8_t>(impl)];
		if (!callback)
			return EError::MissingImpl;
		return callback(input, output, inputSize, outputSize);
	}

	template <class C1, class C2>
	Details::String<C1> auto Convert(Details::StringView<C2> auto str, EImpl impl = EImpl::Fastest)
	{
		constexpr EEncoding From = Details::EncodingTypeV<C2>;
		constexpr EEncoding To   = Details::EncodingTypeV<C1>;

		if constexpr (From == To)
		{
			std::basic_string<C1> result(str.size(), '\0');
			std::memcpy(result.data(), str.data(), str.size() * sizeof(C1));
			return result;
		}
		else
		{
			const void* inputBuf  = str.data();
			std::size_t inputSize = str.size() * sizeof(Details::CharTypeT<From>);
			std::size_t inputOff  = 0;

			std::size_t outputSize = 0;
			EError      error      = CalcReqSize<From, To>(inputBuf, inputSize, outputSize, impl);
			if (error != EError::Success)
				return std::basic_string<C1> {};

			void*       outputBuf = Memory::AlignedMalloc(alignof(OutputBlock), outputSize);
			std::size_t outputOff = 0;
			if (!outputBuf)
				return std::basic_string<C1> {};

			std::size_t firstBytes, lastBytes;
			Details::CalcIters(reinterpret_cast<std::uintptr_t>(inputBuf), inputSize, alignof(InputBlock), firstBytes, lastBytes);
			std::size_t fastIters = (inputSize - firstBytes - lastBytes) / alignof(InputBlock);

			InputBlock  inputBlock;
			OutputBlock outputBlock;
			std::size_t bytesWritten = 0;
			if (firstBytes > 0)
			{
				std::size_t firstPadded = firstBytes;
				if (fastIters + lastBytes > 0)
				{
					if constexpr (From == EEncoding::UTF8)
						firstPadded += std::min<std::size_t>(3, fastIters + lastBytes);
					else if constexpr (From == EEncoding::UTF16)
						firstPadded += std::min<std::size_t>(2, fastIters + lastBytes);
				}
				std::memcpy(&inputBlock, reinterpret_cast<const std::uint8_t*>(inputBuf) + inputOff, firstPadded);
				std::memset(reinterpret_cast<std::uint8_t*>(&inputBlock) + firstPadded, 0, sizeof(inputBlock) - firstPadded);
				error = ConvBlock<From, To>(inputBlock, outputBlock, firstBytes, bytesWritten, impl);
				if (error != EError::Success)
				{
					Memory::AlignedFree(outputBuf, alignof(OutputBlock));
					return std::basic_string<C1> {};
				}
				std::memcpy(reinterpret_cast<std::uint8_t*>(outputBuf) + outputOff, &outputBlock, bytesWritten);
				inputOff  += firstBytes;
				outputOff += bytesWritten;
			}

			for (std::size_t i = 0; i < fastIters; ++i)
			{
				error = ConvBlock<From, To>(*reinterpret_cast<const InputBlock*>(reinterpret_cast<const std::uint8_t*>(inputBuf) + inputOff),
											*reinterpret_cast<OutputBlock*>(reinterpret_cast<std::uint8_t*>(outputBuf) + outputOff),
											alignof(InputBlock),
											bytesWritten,
											impl);
				if (error != EError::Success)
				{
					Memory::AlignedFree(outputBuf, alignof(OutputBlock));
					return std::basic_string<C1> {};
				}
				inputOff  += alignof(InputBlock);
				outputOff += bytesWritten;
			}

			if (lastBytes > 0)
			{
				std::memcpy(&inputBlock, reinterpret_cast<const std::uint8_t*>(inputBuf) + inputOff, lastBytes);
				std::memset(reinterpret_cast<std::uint8_t*>(&inputBlock) + lastBytes, 0, sizeof(inputBlock) - lastBytes);
				error = ConvBlock<From, To>(inputBlock, outputBlock, lastBytes, bytesWritten, impl);
				if (error != EError::Success)
				{
					Memory::AlignedFree(outputBuf, alignof(OutputBlock));
					return std::basic_string<C1> {};
				}
				std::memcpy(reinterpret_cast<std::uint8_t*>(outputBuf) + outputOff, &outputBlock, bytesWritten);
				inputOff  += lastBytes;
				outputOff += bytesWritten;
			}

			std::basic_string<C1> output;
			output.resize(outputSize / sizeof(Details::CharTypeT<To>));
			std::memcpy(output.data(), outputBuf, outputSize);
			Memory::AlignedFree(outputBuf, alignof(OutputBlock));
			return output;
		}
	}

	template <class C1, class C2>
	Details::String<C1> auto Convert(const Details::String<C2> auto& str, EImpl impl = EImpl::Fastest)
	{
		return Convert<C1, C2>(std::basic_string_view<C2>(str), impl);
	}
} // namespace UTF