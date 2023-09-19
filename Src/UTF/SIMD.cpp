#include "UTF/SIMD.h"

namespace UTF::SIMD
{
	EError CalcReqSize8To16([[maybe_unused]] const void* input, [[maybe_unused]] std::size_t inputSize, [[maybe_unused]] std::size_t& requiredSize)
	{
		return EError::MissingImpl;
	}

	EError CalcReqSize8To32([[maybe_unused]] const void* input, [[maybe_unused]] std::size_t inputSize, [[maybe_unused]] std::size_t& requiredSize)
	{
		return EError::MissingImpl;
	}

	EError CalcReqSize16To8([[maybe_unused]] const void* input, [[maybe_unused]] std::size_t inputSize, [[maybe_unused]] std::size_t& requiredSize)
	{
		return EError::MissingImpl;
	}

	EError CalcReqSize16To32([[maybe_unused]] const void* input, [[maybe_unused]] std::size_t inputSize, [[maybe_unused]] std::size_t& requiredSize)
	{
		return EError::MissingImpl;
	}

	EError CalcReqSize32To8([[maybe_unused]] const void* input, [[maybe_unused]] std::size_t inputSize, [[maybe_unused]] std::size_t& requiredSize)
	{
		return EError::MissingImpl;
	}

	EError CalcReqSize32To16([[maybe_unused]] const void* input, [[maybe_unused]] std::size_t inputSize, [[maybe_unused]] std::size_t& requiredSize)
	{
		return EError::MissingImpl;
	}

	EError ConvBlock8To16([[maybe_unused]] const InputBlock& input, [[maybe_unused]] OutputBlock& output, [[maybe_unused]] std::size_t inputSize, [[maybe_unused]] std::size_t& outputSize)
	{
		return EError::MissingImpl;
	}

	EError ConvBlock8To32([[maybe_unused]] const InputBlock& input, [[maybe_unused]] OutputBlock& output, [[maybe_unused]] std::size_t inputSize, [[maybe_unused]] std::size_t& outputSize)
	{
		return EError::MissingImpl;
	}

	EError ConvBlock16To8([[maybe_unused]] const InputBlock& input, [[maybe_unused]] OutputBlock& output, [[maybe_unused]] std::size_t inputSize, [[maybe_unused]] std::size_t& outputSize)
	{
		return EError::MissingImpl;
	}

	EError ConvBlock16To32([[maybe_unused]] const InputBlock& input, [[maybe_unused]] OutputBlock& output, [[maybe_unused]] std::size_t inputSize, [[maybe_unused]] std::size_t& outputSize)
	{
		return EError::MissingImpl;
	}

	EError ConvBlock32To8([[maybe_unused]] const InputBlock& input, [[maybe_unused]] OutputBlock& output, [[maybe_unused]] std::size_t inputSize, [[maybe_unused]] std::size_t& outputSize)
	{
		return EError::MissingImpl;
	}

	EError ConvBlock32To16([[maybe_unused]] const InputBlock& input, [[maybe_unused]] OutputBlock& output, [[maybe_unused]] std::size_t inputSize, [[maybe_unused]] std::size_t& outputSize)
	{
		return EError::MissingImpl;
	}
} // namespace UTF::SIMD
