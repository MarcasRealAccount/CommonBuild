#define TESTING_ENTRYPOINT
#include <Testing/Testing.h>

#undef NDEBUG
#include <cassert>

extern void UTFTests();

struct AssertType
{
};
extern void Thrower();

void RegisterTests()
{
	Testing::PushGroup("Testing");
	Testing::Test("Skip")
		.OnTest([]() { Testing::Skip(); })
		.ExpectResult(Testing::ETestResult::Skip);
	Testing::Test("Fail")
		.OnTest([]() { Testing::Fail(); })
		.ExpectResult(Testing::ETestResult::Fail);
	Testing::Test("Exception test")
		.OnTest([]() { throw 0; })
		.OnException(Testing::ExpectExceptions<int>);
	Testing::Test("Multi Exception test")
		.OnTest([]() { throw 0.0; })
		.OnException(Testing::ExpectExceptions<int, double>);
	Testing::Test("Custom Exception test")
		.OnTest([]() { throw AssertType {}; })
		.OnException(Testing::ExpectExceptions<AssertType>);
	Testing::Test("Custom External Exception test")
		.OnTest([]() { Thrower(); })
		.OnException(Testing::ExpectExceptions<AssertType>);
	Testing::Test("Crash test")
		.OnTest([]() { throw *(volatile int*) 0; })
		.ExpectCrash();
	Testing::Test("Exit test")
		.OnTest([]() { std::exit(0); })
		.ExpectCrash();
	Testing::Test("Terminate test")
		.OnTest([]() { std::terminate(); })
		.ExpectCrash();
	Testing::Test("Assert test")
		.OnTest([]() { assert(false); })
		.ExpectCrash();

	Testing::PushGroup("SubGroup");
	Testing::Test("In SubGroup", []() { Testing::Success(); });
	Testing::PopGroup();

	Testing::Test("After SubGroup", []() { Testing::Success(); });

	Testing::Test("Hidden")
		.OnTest([]() { std::exit(0); })
		.WillCrash()
		.Hide();
	Testing::Test("Skipped")
		.OnTest([]() { Testing::Fail(); })
		.Dependencies("Testing.Hidden")
		.ExpectSkip();
	Testing::Test("Success with crashing OnPostTest")
		.OnTest([]() { Testing::Success(); })
		.OnPostTest([]() { assert(false); })
		.ExpectCrash();
	Testing::PopGroup();

	UTFTests();
}