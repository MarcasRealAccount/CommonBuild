#define TESTING_ENTRYPOINT
#include <Testing/Testing.h>

#include <cassert>
#include <iostream>

extern void UTFTests();

void RegisterTests()
{
	Testing::PushGroup("Testing");
	Testing::Test(
		"Exception test",
		{
			.OnTest = []() {
				throw 0;
			},
			.OnException = Testing::ExpectAnyException,
		});
	Testing::Test(
		"Crash test",
		{
			.OnTest = []() {
				throw *(int*) 0;
			},
			.ExpectCrash = true,
		});
	Testing::Test(
		"Exit test",
		{
			.OnTest = []() {
				std::exit(0);
			},
			.ExpectCrash = true,
		});
	Testing::Test(
		"Terminate test",
		{
			.OnTest = []() {
				std::terminate();
			},
			.ExpectCrash = true,
		});
	Testing::Test(
		"Assert test",
		{
			.OnTest = []() {
				assert(false);
			},
			.ExpectCrash = true,
		});

	Testing::PushGroup("SubGroup");
	Testing::Test("In SubGroup", []() { Testing::Success(); });
	Testing::PopGroup();

	Testing::Test("After SubGroup", []() { Testing::Success(); });

	Testing::Test(
		"Hidden",
		{
			.OnTest    = []() { std::exit(0); },
			.Hidden    = true,
			.WillCrash = true,
		});
	Testing::Test(
		"Skipped",
		{
			.OnTest = []() {
				Testing::Fail();
			},
			.Dependencies = { "Testing.Hidden" },
			.ExpectSkip   = true,
		});
	Testing::PopGroup();

	UTFTests();
}