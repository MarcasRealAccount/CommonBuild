#include <Testing/Testing.h>
#include <UTF/UTF.h>

constexpr const char c_U8Str[]  = "\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xDF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xEF\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF\xF4\x8F\xBF\xBF";
constexpr const char c_U16Str[] = "\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\x7F\x00\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\x07\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\xFF\xDB\xFF\xDF\x00";
constexpr const char c_U32Str[] = "\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\x7F\x00\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\x07\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x00\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\xFF\xFF\x10\x00\x00\x00\x00";

template <UTF::EEncoding From, UTF::EEncoding To, UTF::EImpl Impl>
static void RequiredSizeTest(const void* testString, size_t testStringSize, size_t expectedSize)
{
	size_t requiredSize = 0;
	Testing::Expect(UTF::CalcReqSize<From, To>(testString, testStringSize, requiredSize, Impl) == UTF::EError::Success);
	Testing::Expect(requiredSize == expectedSize);
}

template <UTF::EEncoding From, UTF::EEncoding To, UTF::EImpl Impl>
static void ConvBlockTest(const void* inputStart, size_t inputSize, size_t actualSize, const void* expected, size_t expectedSize)
{
	UTF::InputBlock  input;
	UTF::OutputBlock output;
	memset(&input, 0, sizeof(input));
	memset(&output, 0, sizeof(output));
	memcpy(&input, inputStart, inputSize);
	size_t outputSize = 0;
	Testing::Expect(UTF::ConvBlock<From, To>(input, output, actualSize, outputSize, Impl) == UTF::EError::Success);
	Testing::Expect(outputSize == expectedSize);
	Testing::Expect(memcmp(&output, expected, expectedSize) == 0);
}

template <UTF::EEncoding From, UTF::EEncoding To, UTF::EImpl Impl>
static void ConvTest(const void* testString, size_t testStringSize, const void* expected, size_t expectedSize)
{
	using C1    = UTF::Details::CharTypeT<To>;
	using C2    = UTF::Details::CharTypeT<From>;
	auto input  = std::basic_string_view<C2>(reinterpret_cast<const C2*>(testString), testStringSize / sizeof(C2));
	auto output = std::basic_string_view<C1>(reinterpret_cast<const C1*>(expected), expectedSize / sizeof(C1));

	auto result = UTF::Convert<C1, C2>(input, Impl);
	Testing::Expect(result == output);
}

static void RequiredSizeTests()
{
	Testing::PushGroup("Required Size");

	Testing::PushGroup("Generic");
	Testing::Test("8-16", []() { RequiredSizeTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(c_U8Str, sizeof(c_U8Str) - 1, 144); });
	Testing::Test("8-32", []() { RequiredSizeTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(c_U8Str, sizeof(c_U8Str) - 1, 236); });
	Testing::Test("16-8", []() { RequiredSizeTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(c_U16Str, sizeof(c_U16Str) - 2, 145); });
	Testing::Test("16-32", []() { RequiredSizeTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(c_U16Str, sizeof(c_U16Str) - 2, 236); });
	Testing::Test("32-8", []() { RequiredSizeTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(c_U32Str, sizeof(c_U32Str) - 4, 145); });
	Testing::Test("32-16", []() { RequiredSizeTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(c_U32Str, sizeof(c_U32Str) - 4, 144); });
	Testing::PopGroup();

	if constexpr (UTF::SIMD::c_Supported)
	{
		Testing::PushGroup("SIMD");
		Testing::Test("8-16", []() { RequiredSizeTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(c_U8Str, sizeof(c_U8Str) - 1, 144); });
		Testing::Test("8-32", []() { RequiredSizeTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(c_U8Str, sizeof(c_U8Str) - 1, 236); });
		Testing::Test("16-8", []() { RequiredSizeTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(c_U16Str, sizeof(c_U16Str) - 2, 145); });
		Testing::Test("16-32", []() { RequiredSizeTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(c_U16Str, sizeof(c_U16Str) - 2, 236); });
		Testing::Test("32-8", []() { RequiredSizeTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(c_U32Str, sizeof(c_U32Str) - 4, 145); });
		Testing::Test("32-16", []() { RequiredSizeTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(c_U32Str, sizeof(c_U32Str) - 4, 144); });
		Testing::PopGroup();
	}

	Testing::PopGroup();
}

static void ConvBlockTests()
{
	Testing::PushGroup("Convert Block");

	Testing::PushGroup("Full");

	Testing::PushGroup("Generic");
	Testing::Test("8-16", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(c_U8Str, 67, 64, c_U16Str, 74); });
	Testing::Test("8-32", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(c_U8Str, 67, 64, c_U32Str, 148); });
	Testing::Test("16-8", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(c_U16Str, 66, 64, c_U8Str, 51); });
	Testing::Test("16-32", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(c_U16Str, 66, 64, c_U32Str, 128); });
	Testing::Test("32-8", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(c_U32Str, 64, 64, c_U8Str, 17); });
	Testing::Test("32-16", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(c_U32Str, 64, 64, c_U16Str, 32); });
	Testing::PopGroup();

	if constexpr (UTF::SIMD::c_Supported)
	{
		Testing::PushGroup("SIMD");
		Testing::Test("8-16", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(c_U8Str, 67, 64, c_U16Str, 74); });
		Testing::Test("8-32", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(c_U8Str, 67, 64, c_U32Str, 148); });
		Testing::Test("16-8", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(c_U16Str, 66, 64, c_U8Str, 51); });
		Testing::Test("16-32", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(c_U16Str, 66, 64, c_U32Str, 128); });
		Testing::Test("32-8", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(c_U32Str, 64, 64, c_U8Str, 17); });
		Testing::Test("32-16", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(c_U32Str, 64, 64, c_U16Str, 32); });
		Testing::PopGroup();
	}
	Testing::PopGroup();

	Testing::PushGroup("Partial");

	Testing::PushGroup("Generic");
	Testing::Test("8-16", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(c_U8Str, 13, 10, c_U16Str, 20); });
	Testing::Test("8-32", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(c_U8Str, 13, 10, c_U32Str, 40); });
	Testing::Test("16-8", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(c_U16Str, 12, 10, c_U8Str, 5); });
	Testing::Test("16-32", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(c_U16Str, 12, 10, c_U32Str, 20); });
	Testing::Test("32-8", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(c_U32Str, 10, 10, c_U8Str, 3); });
	Testing::Test("32-16", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(c_U32Str, 10, 10, c_U16Str, 6); });
	Testing::PopGroup();

	if constexpr (UTF::SIMD::c_Supported)
	{
		Testing::PushGroup("SIMD");
		Testing::Test("8-16", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(c_U8Str, 13, 10, c_U16Str, 20); });
		Testing::Test("8-32", []() { ConvBlockTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(c_U8Str, 13, 10, c_U32Str, 40); });
		Testing::Test("16-8", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(c_U16Str, 12, 10, c_U8Str, 5); });
		Testing::Test("16-32", []() { ConvBlockTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(c_U16Str, 12, 10, c_U32Str, 20); });
		Testing::Test("32-8", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(c_U32Str, 10, 10, c_U8Str, 3); });
		Testing::Test("32-16", []() { ConvBlockTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(c_U32Str, 10, 10, c_U16Str, 6); });
		Testing::PopGroup();
	}
	Testing::PopGroup();

	Testing::PopGroup();
}

static void ConvTests()
{
	Testing::PushGroup("Convert");

	Testing::PushGroup("Generic");
	Testing::Test("8-16")
		.OnTest([]() { ConvTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(c_U8Str, sizeof(c_U8Str), c_U16Str, sizeof(c_U16Str)); })
		.Dependencies("UTF.Convert Block.Full.Generic.8-16", "UTF.Convert Block.Partial.Generic.8-16");
	Testing::Test("8-32")
		.OnTest([]() { ConvTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(c_U8Str, sizeof(c_U8Str), c_U32Str, sizeof(c_U32Str)); })
		.Dependencies("UTF.Convert Block.Full.Generic.8-32", "UTF.Convert Block.Partial.Generic.8-32");
	Testing::Test("16-8")
		.OnTest([]() { ConvTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(c_U16Str, sizeof(c_U16Str), c_U8Str, sizeof(c_U8Str)); })
		.Dependencies("UTF.Convert Block.Full.Generic.16-8", "UTF.Convert Block.Partial.Generic.16-8");
	Testing::Test("16-32")
		.OnTest([]() { ConvTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::Generic>(c_U16Str, sizeof(c_U16Str), c_U32Str, sizeof(c_U32Str)); })
		.Dependencies("UTF.Convert Block.Full.Generic.16-32", "UTF.Convert Block.Partial.Generic.16-32");
	Testing::Test("32-8")
		.OnTest([]() { ConvTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::Generic>(c_U32Str, sizeof(c_U32Str), c_U8Str, sizeof(c_U8Str)); })
		.Dependencies("UTF.Convert Block.Full.Generic.32-8", "UTF.Convert Block.Partial.Generic.32-8");
	Testing::Test("32-16")
		.OnTest([]() { ConvTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::Generic>(c_U32Str, sizeof(c_U32Str), c_U16Str, sizeof(c_U16Str)); })
		.Dependencies("UTF.Convert Block.Full.Generic.32-16", "UTF.Convert Block.Partial.Generic.32-16");
	Testing::PopGroup();

	if constexpr (UTF::SIMD::c_Supported)
	{
		Testing::PushGroup("SIMD");
		Testing::Test("8-16")
			.OnTest([]() { ConvTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(c_U8Str, sizeof(c_U8Str), c_U16Str, sizeof(c_U16Str)); })
			.Dependencies("UTF.Convert Block.Full.SIMD.8-16", "UTF.Convert Block.Partial.SIMD.8-16");
		Testing::Test("8-32")
			.OnTest([]() { ConvTest<UTF::EEncoding::UTF8, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(c_U8Str, sizeof(c_U8Str), c_U32Str, sizeof(c_U32Str)); })
			.Dependencies("UTF.Convert Block.Full.SIMD.8-32", "UTF.Convert Block.Partial.SIMD.8-32");
		Testing::Test("16-8")
			.OnTest([]() { ConvTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(c_U16Str, sizeof(c_U16Str), c_U8Str, sizeof(c_U8Str)); })
			.Dependencies("UTF.Convert Block.Full.SIMD.16-8", "UTF.Convert Block.Partial.SIMD.16-8");
		Testing::Test("16-32")
			.OnTest([]() { ConvTest<UTF::EEncoding::UTF16, UTF::EEncoding::UTF32, UTF::EImpl::SIMD>(c_U16Str, sizeof(c_U16Str), c_U32Str, sizeof(c_U32Str)); })
			.Dependencies("UTF.Convert Block.Full.SIMD.16-32", "UTF.Convert Block.Partial.SIMD.16-32");
		Testing::Test("32-8")
			.OnTest([]() { ConvTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF8, UTF::EImpl::SIMD>(c_U32Str, sizeof(c_U32Str), c_U8Str, sizeof(c_U8Str)); })
			.Dependencies("UTF.Convert Block.Full.SIMD.32-8", "UTF.Convert Block.Partial.SIMD.32-8");
		Testing::Test("32-16")
			.OnTest([]() { ConvTest<UTF::EEncoding::UTF32, UTF::EEncoding::UTF16, UTF::EImpl::SIMD>(c_U32Str, sizeof(c_U32Str), c_U16Str, sizeof(c_U16Str)); })
			.Dependencies("UTF.Convert Block.Full.SIMD.32-16", "UTF.Convert Block.Partial.SIMD.32-16");
		Testing::PopGroup();
	}
	Testing::PopGroup();
}

void UTFTests()
{
	Testing::PushGroup("UTF");

	RequiredSizeTests();
	ConvBlockTests();
	ConvTests();

	Testing::PopGroup();
}