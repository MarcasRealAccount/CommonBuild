#pragma once

// INFO: For better compatibility, add '#define TESTING_ENTRYPOINT' to your Main.cpp file, otherwise make sure everything between Testing::Begin() and Testing::End() is at the start of the program lifetime, as this library will duplicate the current process to actually run the tests.
// Additionally, in the process of duplicating the process, the new process will also have the following arguments denoted below appended to the argv array, and requires that you invoke Testing::GetFlagsForArgs to get the required flags and Testing::HandleArgs after Testing::Begin(flags), do note you can pass semi-invalid arguments to those functions, but it is recommended that you handle your own args first, then pass the remainder to those functions.
// * __int_test_runner=<integer>
//
// For a look at how to use the library, have a look at the Tests/ directory.
// #define TESTING_ENTRYPOINT

#include <cstddef>
#include <cstdint>

#include <functional>
#include <string>

namespace Testing
{
	namespace Internal
	{
		template <class T>
		bool ExpectException()
		{
			try
			{
				throw;
			}
			catch ([[maybe_unused]] const T& v)
			{
				return true;
			}
			catch (...)
			{
				return false;
			}
		}
	} // namespace Internal

	static constexpr uint64_t c_IntTestRunner = 0x8000'0000'0000'0000;

	bool     SupportsCrashHandling();
	uint64_t GetFlagsForArgs(size_t argc, const char* const* argv);
	void     Begin(uint64_t flags = 0);
	void     HandleArgs(size_t argc, const char* const* argv);
	void     End();

	void PushGroup(std::string name);
	void PopGroup();

	enum class ETestResult : uint8_t
	{
		NotRun,
		Success,
		Skip,
		Fail,
		Crash,
		TimedOut
	};

	// With the new testing system there's no real need nor ability to use lambda captures.

	using TestFn             = void (*)();
	using ExceptionHandlerFn = bool (*)();

	struct TestSpec
	{
		TestSpec& OnPreTest(TestFn&& onPreTest);
		TestSpec& OnTest(TestFn&& onTest);
		TestSpec& OnPostTest(TestFn&& onPostTest);
		TestSpec& OnException(ExceptionHandlerFn&& onException);
		TestSpec& Dependencies(std::vector<std::string> dependencies);
		template <std::convertible_to<std::string>... Ts>
		TestSpec& Dependencies(Ts&&... dependencies)
		{
			return Dependencies(std::vector<std::string> { std::move(dependencies)... });
		}

		TestSpec& ExpectResult(ETestResult result);
		TestSpec& ExpectSkip() { return ExpectResult(ETestResult::Skip); }
		TestSpec& ExpectCrash() { return ExpectResult(ETestResult::Crash); }

		TestSpec& Hide();
		TestSpec& WillCrash();

		TestSpec& Time(double baselineTime = 0.0);
	};

	struct TestDesc
	{
		TestFn                   OnPreTest;
		TestFn                   OnTest;
		TestFn                   OnPostTest;
		std::vector<std::string> Dependencies;
		ExceptionHandlerFn       OnException;

		bool ExpectCrash = false;
		bool ExpectSkip  = false;
		bool Hidden      = false;
		bool WillCrash   = false;

		bool   Timed        = false;
		double BaselineTime = 0.0;
	};

	TestSpec&        Test(std::string name);
	inline TestSpec& Test(std::string name, TestDesc desc)
	{
		auto& spec = Test(std::move(name))
						 .OnPreTest(std::move(desc.OnPreTest))
						 .OnTest(std::move(desc.OnTest))
						 .OnPostTest(std::move(desc.OnPostTest))
						 .OnException(std::move(desc.OnException))
						 .Dependencies(std::move(desc.Dependencies));
		if (desc.ExpectCrash)
			spec.ExpectCrash();
		if (desc.ExpectSkip)
			spec.ExpectSkip();
		if (desc.Hidden)
			spec.Hide();
		if (desc.WillCrash)
			spec.WillCrash();
		if (desc.Timed)
			spec.Time(desc.BaselineTime);
		return spec;
	}
	inline TestSpec& Test(std::string name, TestFn&& onTest)
	{
		return Test(name).OnTest(std::move(onTest));
	}

	void Expect(bool expectation);
	void Success();
	void Skip();
	void Fail();

	inline bool ExpectAnyException()
	{
		return true;
	}
	template <class... ExceptionTypes>
	bool ExpectExceptions()
	{
		return (false || ... || Internal::ExpectException<ExceptionTypes>());
	}
} // namespace Testing

#ifdef TESTING_ENTRYPOINT
void RegisterTests();

int main(int argc, char** argv)
{
	uint64_t flags = Testing::GetFlagsForArgs(argc, argv);
	Testing::Begin(flags);
	Testing::HandleArgs(argc, argv);
	RegisterTests();
	Testing::End();
}
#endif