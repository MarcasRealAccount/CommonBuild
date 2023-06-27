#pragma once

#include "Base.h"

namespace UTF::Generic
{
	EError CalcReqSize8To16(const void* input, std::size_t inputSize, std::size_t& requiredSize);
	EError CalcReqSize8To32(const void* input, std::size_t inputSize, std::size_t& requiredSize);
	EError CalcReqSize16To8(const void* input, std::size_t inputSize, std::size_t& requiredSize);
	EError CalcReqSize16To32(const void* input, std::size_t inputSize, std::size_t& requiredSize);
	EError CalcReqSize32To8(const void* input, std::size_t inputSize, std::size_t& requiredSize);
	EError CalcReqSize32To16(const void* input, std::size_t inputSize, std::size_t& requiredSize);

	EError ConvBlock8To16(const InputBlock& input, OutputBlock& output, std::size_t inputSize, std::size_t& outputSize);
	EError ConvBlock8To32(const InputBlock& input, OutputBlock& output, std::size_t inputSize, std::size_t& outputSize);
	EError ConvBlock16To8(const InputBlock& input, OutputBlock& output, std::size_t inputSize, std::size_t& outputSize);
	EError ConvBlock16To32(const InputBlock& input, OutputBlock& output, std::size_t inputSize, std::size_t& outputSize);
	EError ConvBlock32To8(const InputBlock& input, OutputBlock& output, std::size_t inputSize, std::size_t& outputSize);
	EError ConvBlock32To16(const InputBlock& input, OutputBlock& output, std::size_t inputSize, std::size_t& outputSize);
} // namespace UTF::Generic