#include <Testing/Testing.h>
#include <UTF/UTF.h>

#include <cstring>

#include <string>
#include <string_view>
#include <thread>

constexpr const char s_U8Str[]  = "\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF";
constexpr const char s_U16Str[] = "\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\x00";
constexpr const char s_U32Str[] = "\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\x00\x00\x00";

static UTF::InputBlock s_U8FullIn;
static UTF::InputBlock s_U8PartialIn;
static UTF::InputBlock s_U16FullIn;
static UTF::InputBlock s_U16PartialIn;
static UTF::InputBlock s_U32FullIn;
static UTF::InputBlock s_U32PartialIn;

template <UTF::EEncoding From, UTF::EEncoding To, UTF::EImpl Impl>
void RequiredSizeTest(const void* testString, std::size_t testStringSize, std::size_t expectedSize)
{
	std::size_t requiredSize = 0;
	UTF::EError error        = UTF::CalcReqSize<From, To>(testString, testStringSize, requiredSize, Impl);
	Testing::Assert(error == UTF::EError::Success && requiredSize == expectedSize);
}

template <UTF::EEncoding From, UTF::EEncoding To, UTF::EImpl Impl>
void ConvBlockTest(const UTF::InputBlock& input, std::size_t inputSize, const void* expected, std::size_t expectedSize)
{
	UTF::OutputBlock output;
	std::size_t      outputSize = 0;
	UTF::EError      error      = UTF::ConvBlock<From, To>(input, output, inputSize, outputSize, Impl);
	Testing::Assert(error == UTF::EError::Success && outputSize == expectedSize);
	Testing::Assert(std::memcmp(&output, expected, expectedSize) == 0);
}

template <UTF::EEncoding From, UTF::EEncoding To, UTF::EImpl Impl>
void Conv(const void* testString, std::size_t testStringSize, const void* expected, std::size_t expectedSize)
{
	using C1    = UTF::Details::CharTypeT<To>;
	using C2    = UTF::Details::CharTypeT<From>;
	auto input  = std::basic_string_view<C2>(reinterpret_cast<const C2*>(testString), testStringSize / sizeof(C2));
	auto output = std::basic_string_view<C1>(reinterpret_cast<const C1*>(expected), expectedSize / sizeof(C1));

	auto result = UTF::Convert<C1, C2>(input, Impl);
	Testing::Assert(result == output);
}

void UTFTests()
{
	std::memcpy(&s_U8FullIn, s_U8Str, 67);
	std::memcpy(&s_U16FullIn, s_U16Str, 66);
	std::memcpy(&s_U32FullIn, s_U32Str, 64);
	std::memset(&s_U8PartialIn, 0, sizeof(UTF::InputBlock));
	std::memset(&s_U16PartialIn, 0, sizeof(UTF::InputBlock));
	std::memset(&s_U32PartialIn, 0, sizeof(UTF::InputBlock));
	std::memcpy(&s_U8PartialIn, s_U8Str, 13);
	std::memcpy(&s_U16PartialIn, s_U16Str, 12);
	std::memcpy(&s_U32PartialIn, s_U32Str, 10);

	{
		Testing::PushGroup("UTF Required Size Generic");
		Testing::Test("UTF8 To UTF16", []() { RequiredSizeTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U8Str, sizeof(s_U8Str) - 1, 144); });
		Testing::Test("UTF8 To UTF32", []() { RequiredSizeTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U8Str, sizeof(s_U8Str) - 1, 236); });
		Testing::Test("UTF16 To UTF8", []() { RequiredSizeTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U16Str, sizeof(s_U16Str) - 2, 145); });
		Testing::Test("UTF16 To UTF32", []() { RequiredSizeTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U16Str, sizeof(s_U16Str) - 2, 236); });
		Testing::Test("UTF32 To UTF8", []() { RequiredSizeTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U32Str, sizeof(s_U32Str) - 4, 145); });
		Testing::Test("UTF32 To UTF16", []() { RequiredSizeTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U32Str, sizeof(s_U32Str) - 4, 144); });
		Testing::PopGroup();

		if constexpr (UTF::SIMD::c_Supported)
		{
			Testing::PushGroup("UTF Required Size SIMD");
			Testing::Test("UTF8 To UTF16", []() { RequiredSizeTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U8Str, sizeof(s_U8Str) - 1, 144); });
			Testing::Test("UTF8 To UTF32", []() { RequiredSizeTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U8Str, sizeof(s_U8Str) - 1, 236); });
			Testing::Test("UTF16 To UTF8", []() { RequiredSizeTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U16Str, sizeof(s_U16Str) - 2, 145); });
			Testing::Test("UTF16 To UTF32", []() { RequiredSizeTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U16Str, sizeof(s_U16Str) - 2, 236); });
			Testing::Test("UTF32 To UTF8", []() { RequiredSizeTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U32Str, sizeof(s_U32Str) - 4, 145); });
			Testing::Test("UTF32 To UTF16", []() { RequiredSizeTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U32Str, sizeof(s_U32Str) - 4, 144); });
			Testing::PopGroup();
		}
	}

	{
		Testing::PushGroup("UTF Convert Block Full Generic");
		Testing::Test("UTF8 To UTF16", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U8FullIn, 64, s_U16Str, 74); });
		Testing::Test("UTF8 To UTF32", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U8FullIn, 64, s_U32Str, 148); });
		Testing::Test("UTF16 To UTF8", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U16FullIn, 64, s_U8Str, 51); });
		Testing::Test("UTF16 To UTF32", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U16FullIn, 64, s_U32Str, 128); });
		Testing::Test("UTF32 To UTF8", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U32FullIn, 64, s_U8Str, 17); });
		Testing::Test("UTF32 To UTF16", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U32FullIn, 64, s_U16Str, 32); });
		Testing::PopGroup();

		Testing::PushGroup("UTF Convert Block Partial Generic");
		Testing::Test("UTF8 To UTF16", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U8PartialIn, 10, s_U16Str, 20); });
		Testing::Test("UTF8 To UTF32", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U8PartialIn, 10, s_U32Str, 40); });
		Testing::Test("UTF16 To UTF8", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U16PartialIn, 10, s_U8Str, 5); });
		Testing::Test("UTF16 To UTF32", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U16PartialIn, 10, s_U32Str, 20); });
		Testing::Test("UTF32 To UTF8", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U32PartialIn, 10, s_U8Str, 3); });
		Testing::Test("UTF32 To UTF16", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U32PartialIn, 10, s_U16Str, 6); });
		Testing::PopGroup();

		if constexpr (UTF::SIMD::c_Supported)
		{
			Testing::PushGroup("UTF Convert Block Full SIMD");
			Testing::Test("UTF8 To UTF16", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U8FullIn, 64, s_U16Str, 74); });
			Testing::Test("UTF8 To UTF32", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U8FullIn, 64, s_U32Str, 148); });
			Testing::Test("UTF16 To UTF8", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U16FullIn, 64, s_U8Str, 51); });
			Testing::Test("UTF16 To UTF32", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U16FullIn, 64, s_U32Str, 128); });
			Testing::Test("UTF32 To UTF8", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U32FullIn, 64, s_U8Str, 17); });
			Testing::Test("UTF32 To UTF16", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U32FullIn, 64, s_U16Str, 32); });
			Testing::PopGroup();

			Testing::PushGroup("UTF Convert Block Partial SIMD");
			Testing::Test("UTF8 To UTF16", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U8PartialIn, 10, s_U16Str, 20); });
			Testing::Test("UTF8 To UTF32", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U8PartialIn, 10, s_U32Str, 40); });
			Testing::Test("UTF16 To UTF8", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U16PartialIn, 10, s_U8Str, 5); });
			Testing::Test("UTF16 To UTF32", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U16PartialIn, 10, s_U32Str, 20); });
			Testing::Test("UTF32 To UTF8", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U32PartialIn, 10, s_U8Str, 3); });
			Testing::Test("UTF32 To UTF16", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U32PartialIn, 10, s_U16Str, 6); });
			Testing::PopGroup();
		}
	}

	{
		Testing::PushGroup("UTF Convert Generic");
		Testing::Test("UTF8 To UTF16", []() { Conv<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U8Str, sizeof(s_U8Str), s_U16Str, sizeof(s_U16Str)); });
		Testing::Test("UTF8 To UTF32", []() { Conv<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U8Str, sizeof(s_U8Str), s_U32Str, sizeof(s_U32Str)); });
		Testing::Test("UTF16 To UTF8", []() { Conv<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U16Str, sizeof(s_U16Str), s_U8Str, sizeof(s_U8Str)); });
		Testing::Test("UTF16 To UTF32", []() { Conv<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U16Str, sizeof(s_U16Str), s_U32Str, sizeof(s_U32Str)); });
		Testing::Test("UTF32 To UTF16", []() { Conv<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U32Str, sizeof(s_U32Str), s_U8Str, sizeof(s_U8Str)); });
		Testing::Test("UTF32 To UTF32", []() { Conv<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U32Str, sizeof(s_U32Str), s_U16Str, sizeof(s_U16Str)); });
		Testing::PopGroup();

		if constexpr (UTF::SIMD::c_Supported)
		{
			Testing::PushGroup("UTF Convert SIMD");
			Testing::Test("UTF8 To UTF16", []() { Conv<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U8Str, sizeof(s_U8Str), s_U16Str, sizeof(s_U16Str)); });
			Testing::Test("UTF8 To UTF32", []() { Conv<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U8Str, sizeof(s_U8Str), s_U32Str, sizeof(s_U32Str)); });
			Testing::Test("UTF16 To UTF8", []() { Conv<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U16Str, sizeof(s_U16Str), s_U8Str, sizeof(s_U8Str)); });
			Testing::Test("UTF16 To UTF32", []() { Conv<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U16Str, sizeof(s_U16Str), s_U32Str, sizeof(s_U32Str)); });
			Testing::Test("UTF32 To UTF16", []() { Conv<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U32Str, sizeof(s_U32Str), s_U8Str, sizeof(s_U8Str)); });
			Testing::Test("UTF32 To UTF32", []() { Conv<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U32Str, sizeof(s_U32Str), s_U16Str, sizeof(s_U16Str)); });
			Testing::PopGroup();
		}
	}

	{
		double reqSizeBaselines[12];
		reqSizeBaselines[0] = 10'000 * (sizeof(s_U8Str) - 1);
		reqSizeBaselines[1] = 10'000 * (sizeof(s_U8Str) - 1);
		reqSizeBaselines[2] = 10'000 * (sizeof(s_U16Str) - 1);
		reqSizeBaselines[3] = 10'000 * (sizeof(s_U16Str) - 1);
		reqSizeBaselines[4] = 10'000 * (sizeof(s_U32Str) - 1);
		reqSizeBaselines[5] = 10'000 * (sizeof(s_U32Str) - 1);

		Testing::PushGroup("UTF Required Size Timed Generic");
		Testing::TimedTest("UTF8 To UTF16", reqSizeBaselines[0], []() { RequiredSizeTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U8Str, sizeof(s_U8Str) - 1, 144); });
		Testing::TimedTest("UTF8 To UTF32", reqSizeBaselines[1], []() { RequiredSizeTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U8Str, sizeof(s_U8Str) - 1, 236); });
		Testing::TimedTest("UTF16 To UTF8", reqSizeBaselines[2], []() { RequiredSizeTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U16Str, sizeof(s_U16Str) - 2, 145); });
		Testing::TimedTest("UTF16 To UTF32", reqSizeBaselines[3], []() { RequiredSizeTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U16Str, sizeof(s_U16Str) - 2, 236); });
		Testing::TimedTest("UTF32 To UTF8", reqSizeBaselines[4], []() { RequiredSizeTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U32Str, sizeof(s_U32Str) - 4, 145); });
		Testing::TimedTest("UTF32 To UTF16", reqSizeBaselines[5], []() { RequiredSizeTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U32Str, sizeof(s_U32Str) - 4, 144); });
		Testing::PopGroup();

		if constexpr (UTF::SIMD::c_Supported)
		{
			reqSizeBaselines[6]  = 500 * (sizeof(s_U8Str) - 1);
			reqSizeBaselines[7]  = 500 * (sizeof(s_U8Str) - 1);
			reqSizeBaselines[8]  = 500 * (sizeof(s_U16Str) - 1);
			reqSizeBaselines[9]  = 500 * (sizeof(s_U16Str) - 1);
			reqSizeBaselines[10] = 500 * (sizeof(s_U32Str) - 1);
			reqSizeBaselines[11] = 500 * (sizeof(s_U32Str) - 1);
			Testing::PushGroup("UTF Required Size Timed SIMD");
			Testing::TimedTest("UTF8 To UTF16", reqSizeBaselines[6], []() { RequiredSizeTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U8Str, sizeof(s_U8Str) - 1, 144); });
			Testing::TimedTest("UTF8 To UTF32", reqSizeBaselines[7], []() { RequiredSizeTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U8Str, sizeof(s_U8Str) - 1, 236); });
			Testing::TimedTest("UTF16 To UTF8", reqSizeBaselines[8], []() { RequiredSizeTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U16Str, sizeof(s_U16Str) - 2, 145); });
			Testing::TimedTest("UTF16 To UTF32", reqSizeBaselines[9], []() { RequiredSizeTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U16Str, sizeof(s_U16Str) - 2, 236); });
			Testing::TimedTest("UTF32 To UTF8", reqSizeBaselines[10], []() { RequiredSizeTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U32Str, sizeof(s_U32Str) - 4, 145); });
			Testing::TimedTest("UTF32 To UTF16", reqSizeBaselines[11], []() { RequiredSizeTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U32Str, sizeof(s_U32Str) - 4, 144); });
			Testing::PopGroup();
		}
	}

	{
		double baselines[24];
		baselines[0] = 15'000 * 37;
		baselines[1] = 15'000 * 37;
		baselines[2] = 15'000 * 32;
		baselines[3] = 15'000 * 32;
		baselines[4] = 15'000 * 16;
		baselines[5] = 15'000 * 16;

		baselines[6]  = 15'000 * 10;
		baselines[7]  = 15'000 * 10;
		baselines[8]  = 15'000 * 5;
		baselines[9]  = 15'000 * 5;
		baselines[10] = 15'000 * 3;
		baselines[11] = 15'000 * 3;

		Testing::PushGroup("UTF Convert Block Timed Full Generic");
		Testing::TimedTest("UTF8 To UTF16", baselines[0], []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U8FullIn, 64, s_U16Str, 74); });
		Testing::TimedTest("UTF8 To UTF32", baselines[1], []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U8FullIn, 64, s_U32Str, 148); });
		Testing::TimedTest("UTF16 To UTF8", baselines[2], []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U16FullIn, 64, s_U8Str, 51); });
		Testing::TimedTest("UTF16 To UTF32", baselines[3], []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U16FullIn, 64, s_U32Str, 128); });
		Testing::TimedTest("UTF32 To UTF8", baselines[4], []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U32FullIn, 64, s_U8Str, 17); });
		Testing::TimedTest("UTF32 To UTF16", baselines[5], []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U32FullIn, 64, s_U16Str, 32); });
		Testing::PopGroup();

		Testing::PushGroup("UTF Convert Block Timed Partial Generic");
		Testing::TimedTest("UTF8 To UTF16", baselines[6], []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U8PartialIn, 10, s_U16Str, 20); });
		Testing::TimedTest("UTF8 To UTF32", baselines[7], []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U8PartialIn, 10, s_U32Str, 40); });
		Testing::TimedTest("UTF16 To UTF8", baselines[8], []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U16PartialIn, 10, s_U8Str, 5); });
		Testing::TimedTest("UTF16 To UTF32", baselines[9], []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U16PartialIn, 10, s_U32Str, 20); });
		Testing::TimedTest("UTF32 To UTF8", baselines[10], []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U32PartialIn, 10, s_U8Str, 3); });
		Testing::TimedTest("UTF32 To UTF16", baselines[11], []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U32PartialIn, 10, s_U16Str, 6); });
		Testing::PopGroup();

		if constexpr (UTF::SIMD::c_Supported)
		{
			baselines[12] = 750 * 37;
			baselines[13] = 750 * 37;
			baselines[14] = 750 * 32;
			baselines[15] = 750 * 32;
			baselines[16] = 750 * 16;
			baselines[17] = 750 * 16;

			baselines[18] = 750 * 10;
			baselines[19] = 750 * 10;
			baselines[20] = 750 * 5;
			baselines[21] = 750 * 5;
			baselines[22] = 750 * 3;
			baselines[23] = 750 * 3;

			Testing::PushGroup("UTF Convert Block Timed Full SIMD");
			Testing::TimedTest("UTF8 To UTF16", baselines[12], []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U8FullIn, 64, s_U16Str, 74); });
			Testing::TimedTest("UTF8 To UTF32", baselines[13], []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U8FullIn, 64, s_U32Str, 148); });
			Testing::TimedTest("UTF16 To UTF8", baselines[14], []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U16FullIn, 64, s_U8Str, 51); });
			Testing::TimedTest("UTF16 To UTF32", baselines[15], []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U16FullIn, 64, s_U32Str, 128); });
			Testing::TimedTest("UTF32 To UTF8", baselines[16], []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U32FullIn, 64, s_U8Str, 17); });
			Testing::TimedTest("UTF32 To UTF16", baselines[17], []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U32FullIn, 64, s_U16Str, 32); });
			Testing::PopGroup();

			Testing::PushGroup("UTF Convert Block Timed Partial SIMD");
			Testing::TimedTest("UTF8 To UTF16", baselines[18], []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U8PartialIn, 10, s_U16Str, 20); });
			Testing::TimedTest("UTF8 To UTF32", baselines[19], []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U8PartialIn, 10, s_U32Str, 40); });
			Testing::TimedTest("UTF16 To UTF8", baselines[20], []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U16PartialIn, 10, s_U8Str, 5); });
			Testing::TimedTest("UTF16 To UTF32", baselines[21], []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U16PartialIn, 10, s_U32Str, 20); });
			Testing::TimedTest("UTF32 To UTF8", baselines[22], []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U32PartialIn, 10, s_U8Str, 3); });
			Testing::TimedTest("UTF32 To UTF16", baselines[23], []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U32PartialIn, 10, s_U16Str, 6); });
			Testing::PopGroup();
		}
	}

	{
		double baselines[12];
		baselines[0] = 15'000 * sizeof(s_U8Str);
		baselines[1] = 15'000 * sizeof(s_U8Str);
		baselines[2] = 15'000 * sizeof(s_U16Str);
		baselines[3] = 15'000 * sizeof(s_U16Str);
		baselines[4] = 15'000 * sizeof(s_U32Str);
		baselines[5] = 15'000 * sizeof(s_U32Str);

		baselines[6]  = 750 * sizeof(s_U8Str);
		baselines[7]  = 750 * sizeof(s_U8Str);
		baselines[8]  = 750 * sizeof(s_U16Str);
		baselines[9]  = 750 * sizeof(s_U16Str);
		baselines[10] = 750 * sizeof(s_U32Str);
		baselines[11] = 750 * sizeof(s_U32Str);

		Testing::PushGroup("UTF Convert Generic");
		Testing::TimedTest("UTF8 To UTF16", baselines[0], []() { Conv<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U8Str, sizeof(s_U8Str), s_U16Str, sizeof(s_U16Str)); });
		Testing::TimedTest("UTF8 To UTF32", baselines[1], []() { Conv<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U8Str, sizeof(s_U8Str), s_U32Str, sizeof(s_U32Str)); });
		Testing::TimedTest("UTF16 To UTF8", baselines[2], []() { Conv<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U16Str, sizeof(s_U16Str), s_U8Str, sizeof(s_U8Str)); });
		Testing::TimedTest("UTF16 To UTF32", baselines[3], []() { Conv<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(s_U16Str, sizeof(s_U16Str), s_U32Str, sizeof(s_U32Str)); });
		Testing::TimedTest("UTF32 To UTF16", baselines[4], []() { Conv<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(s_U32Str, sizeof(s_U32Str), s_U8Str, sizeof(s_U8Str)); });
		Testing::TimedTest("UTF32 To UTF32", baselines[5], []() { Conv<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(s_U32Str, sizeof(s_U32Str), s_U16Str, sizeof(s_U16Str)); });
		Testing::PopGroup();

		if constexpr (UTF::SIMD::c_Supported)
		{
			Testing::PushGroup("UTF Convert SIMD");
			Testing::TimedTest("UTF8 To UTF16", baselines[6], []() { Conv<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U8Str, sizeof(s_U8Str), s_U16Str, sizeof(s_U16Str)); });
			Testing::TimedTest("UTF8 To UTF32", baselines[7], []() { Conv<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U8Str, sizeof(s_U8Str), s_U32Str, sizeof(s_U32Str)); });
			Testing::TimedTest("UTF16 To UTF8", baselines[8], []() { Conv<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U16Str, sizeof(s_U16Str), s_U8Str, sizeof(s_U8Str)); });
			Testing::TimedTest("UTF16 To UTF32", baselines[9], []() { Conv<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(s_U16Str, sizeof(s_U16Str), s_U32Str, sizeof(s_U32Str)); });
			Testing::TimedTest("UTF32 To UTF16", baselines[10], []() { Conv<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(s_U32Str, sizeof(s_U32Str), s_U8Str, sizeof(s_U8Str)); });
			Testing::TimedTest("UTF32 To UTF32", baselines[11], []() { Conv<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(s_U32Str, sizeof(s_U32Str), s_U16Str, sizeof(s_U16Str)); });
			Testing::PopGroup();
		}
	}
}