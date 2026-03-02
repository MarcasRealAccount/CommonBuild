#include "Testing/Testing.h"
#include "State.h"
#include "TestRunner.h"

#include <cassert>
#include <cwchar>

#include <chrono>
#include <format>
#include <iostream>

namespace Testing
{
	State* g_State = nullptr;

	void RunTest(TestState& test)
	{
		if (test.Result.Result != ETestResult::NotRun)
			return;

#if SUPPORT_SEPARATE_TEST_RUNNER
		if (g_State->Flags & c_NoTestRunner)
#endif
		{
			if (test.ExpectedResult == ETestResult::Crash || test.WillCrash)
			{
				test.Result.Result = ETestResult::Fail;
				return;
			}
		}

		if (test.OnPreTest)
			test.OnPreTest();

		try
		{
			test.OnTest();
			test.Result.Result = ETestResult::Success;
		}
		catch (TestResult result)
		{
			switch (result.Result)
			{
			case ETestResult::Skip:
			case ETestResult::Fail:
				test.Result = result;
				break;
			default:
				test.Result.Result = result.Result;
				break;
			}
		}
		catch (...)
		{
			std::exception_ptr ep = std::current_exception();
			if (test.OnException && test.OnException(ep))
				test.Result.Result = ETestResult::Success;
			else
				test.Result.Result = ETestResult::Crash;
		}

		if (test.OnPostTest)
			test.OnPostTest();
	}

	void RunTimedTest(TestState& test)
	{
		using Clock = std::chrono::steady_clock;
		using namespace std::chrono_literals;

		// It doesn't make sense to time broken functions. We should only time tests that expect functional success.
		if ((test.ExpectedResult != ETestResult::NotRun && test.ExpectedResult != ETestResult::Success) || test.WillCrash || test.OnException)
		{
			test.Time = -1.0;
			return;
		}

		if (test.OnPreTest)
			test.OnPreTest();

		size_t iters     = 0;
		auto   begin     = Clock::now();
		auto   targetEnd = begin + (g_State->TimedMaxDuration * 1s);
		auto   cur       = begin;
		while (cur < targetEnd && iters < 1000)
		{
			test.OnTest();
			cur = Clock::now();
			++iters;
		}

		test.Time = std::chrono::duration_cast<std::chrono::duration<double>>(cur - begin).count() / iters;

		if (test.OnPostTest)
			test.OnPostTest();
	}

	static std::vector<size_t> GetGroupPath(size_t id)
	{
		std::vector<size_t> path;
		while (id != ~size_t(0))
		{
			path.emplace_back(id);
			id = g_State->Groups[id].Parent;
		}
		std::reverse(path.begin(), path.end());
		return path;
	}

	static size_t OutputGroupChange(size_t from, size_t to)
	{
		auto fromPath = GetGroupPath(from);
		if (from == to)
			return fromPath.size();
		auto toPath = GetGroupPath(to);

		size_t shared = 0;
		while (shared < fromPath.size() && shared < toPath.size() && fromPath[shared] == toPath[shared])
			++shared;

		for (size_t i = fromPath.size(); i-- > shared;)
		{
			auto& group = g_State->Groups[fromPath[i]];

			char colorI;
			if (group.Success == group.Total)
				colorI = '2';
			else
			{
				double factor = (double) group.Success / group.Total;
				if (factor < 0.5)
					colorI = '1';
				else
					colorI = '3';
			}

			std::cout << std::format("{:{}}--- \033[3{}m{}/{}\033[39m ---\n", "", 2 * i, colorI, group.Success, group.Total);
		}

		for (size_t i = shared; i < toPath.size(); ++i)
		{
			auto& group = g_State->Groups[toPath[i]];

			std::cout << std::format("{:{}}--- \033[36m{}\033[39m ---\n", "", 2 * i, group.Name);
		}

		return toPath.size();
	}

	static void PrettyFormatTime(double time, char& unit, uint16_t& whole, uint16_t& decimal)
	{
		if (time == 0.0)
		{
			unit    = ' ';
			whole   = 0;
			decimal = 0;
			return;
		}

		constexpr const char c_Units[] = "qryzafpnum kMGTPEZYRQ";

		int8_t exponent = 0;
		while (time >= 1000.0)
		{
			++exponent;
			time /= 1000.0;
		}
		while (time < 1.0)
		{
			--exponent;
			time *= 1000.0;
		}

		whole   = (uint16_t) time;
		time   -= whole;
		time   *= 1000.0;
		decimal = (uint16_t) time;

		exponent += 10;
		if (exponent < 0 || (size_t) exponent >= sizeof(c_Units) - 1)
			unit = '?';
		else
			unit = c_Units[exponent];
	}

	void OutputTestResult(TestState& test)
	{
		if (test.Hidden)
			return;

		if (test.Result.Result == ETestResult::Success)
		{
			auto cur = test.Group;
			while (cur != ~size_t(0))
			{
				++g_State->Groups[cur].Success;
				cur = g_State->Groups[cur].Parent;
			}
		}
		if (g_State->IntCurOutputGroup != test.Group)
		{
			g_State->IntCurGroupDepth  = OutputGroupChange(g_State->IntCurOutputGroup, test.Group);
			g_State->IntCurOutputGroup = test.Group;
		}
		std::string_view resultStr;
		switch (test.Result.Result)
		{
		case ETestResult::Success: resultStr = "  \033[32mSuccess\033[39m"; break;
		case ETestResult::Skip: resultStr = "     \033[33mSkip\033[39m"; break;
		case ETestResult::Fail: resultStr = "     \033[31mFail\033[39m"; break;
		case ETestResult::Crash: resultStr = "    \033[31mCrash\033[39m"; break;
		case ETestResult::TimedOut: resultStr = "\033[34mTimed Out\033[39m"; break;
		default: resultStr = "     \033[31mFAIL\033[39m"; break;
		}
		if (test.Result.Result != ETestResult::Fail)
		{
			if (test.Timed && test.Result.Result == ETestResult::Success && test.Time >= 0.0)
			{
				char     timeUnit;
				uint16_t timeWhole;
				uint16_t timeDecimal;
				PrettyFormatTime(test.Time, timeUnit, timeWhole, timeDecimal);
				if (test.BaselineTime > 0.0)
				{
					char timeColorI;
					if (test.Time < test.BaselineTime)
						timeColorI = '2';
					else if (test.Time > test.BaselineTime * 1.2)
						timeColorI = '1';
					else
						timeColorI = '3';

					char     baselineUnit;
					uint16_t baselineWhole;
					uint16_t baselineDecimal;
					PrettyFormatTime(test.BaselineTime, baselineUnit, baselineWhole, baselineDecimal);
					if (test.BaseCount > 0)
					{
						char     speedUnit;
						uint16_t speedWhole;
						uint16_t speedDecimal;
						PrettyFormatTime(test.BaseCount / test.Time, speedUnit, speedWhole, speedDecimal);
						std::cout << std::format("{:{}}{} (\033[3{}m{:3}.{:03}{}s/{:3}.{:03}{}s => {:3}.{:03}{}{}/s\033[39m): {}\n", "", 2 * g_State->IntCurGroupDepth, resultStr, timeColorI, timeWhole, timeDecimal, timeUnit, baselineWhole, baselineDecimal, baselineUnit, speedWhole, speedDecimal, speedUnit, test.TimeUnit, test.Name);
					}
					else
					{
						std::cout << std::format("{:{}}{} (\033[3{}m{:3}.{:03}{}s/{:3}.{:03}{}s\033[39m): {}\n", "", 2 * g_State->IntCurGroupDepth, resultStr, timeColorI, timeWhole, timeDecimal, timeUnit, baselineWhole, baselineDecimal, baselineUnit, test.Name);
					}
				}
				else if (test.BaseCount > 0)
				{
					char     speedUnit;
					uint16_t speedWhole;
					uint16_t speedDecimal;
					PrettyFormatTime(test.BaseCount / test.Time, speedUnit, speedWhole, speedDecimal);
					std::cout << std::format("{:{}}{} (\033[32m{:3}.{:03}{}s => {:3}.{:03}{}{}/s\033[39m): {}\n", "", 2 * g_State->IntCurGroupDepth, resultStr, timeWhole, timeDecimal, timeUnit, speedWhole, speedDecimal, speedUnit, test.TimeUnit, test.Name);
				}
				else
				{
					std::cout << std::format("{:{}}{} (\033[32m{:3}.{:03}{}s\033[39m): {}\n", "", 2 * g_State->IntCurGroupDepth, resultStr, timeWhole, timeDecimal, timeUnit, test.Name);
				}
			}
			else
			{
				if (g_State->Flags & c_Compact)
					return;

				std::cout << std::format("{:{}}{}: {}\n", "", 2 * g_State->IntCurGroupDepth, resultStr, test.Name);
			}
		}
		else
		{
			std::cout << std::format("{:{}}{}: {} ({})\n", "", 2 * g_State->IntCurGroupDepth, resultStr, test.Name, test.Result.Location);
		}
	}

	// #if !SUPPORT_SEPARATE_TEST_RUNNER
	static void RunTestRecursive(TestState& test)
	{
		if (test.Result.Result != ETestResult::NotRun)
			return;

		bool skip = false;
		for (auto& dependency : test.Dependencies)
		{
			auto itr = g_State->IntTestToID.find(dependency);
			if (itr == g_State->IntTestToID.end())
			{
				std::cerr << std::format("ERROR: '{}' is not a valid test name\n", dependency);
				skip = true;
				break;
			}

			auto& depTest = g_State->Tests[itr->second];
			RunTestRecursive(depTest);
			if (depTest.ExpectedResult == depTest.Result.Result)
				depTest.Result.Result = ETestResult::Success;
			skip = skip || (depTest.Result.Result != ETestResult::Success);
		}

		if (!test.Hidden)
		{
			auto cur = test.Group;
			while (cur != ~size_t(0))
			{
				++g_State->Groups[cur].Total;
				cur = g_State->Groups[cur].Parent;
			}
		}

		if (skip)
		{
			test.Result.Result = ETestResult::Skip;
			return;
		}
		RunTest(test);

		if (test.WillCrash)
			return;

		if (test.Timed && test.Result.Result == ETestResult::Success)
			RunTimedTest(test);
	}

	static void RunTests()
	{
		for (auto failed : g_State->IntTestsFailed)
			g_State->Tests[failed].Result.Result = ETestResult::Fail;

		for (size_t i = g_State->IntTestsStart; i < g_State->Tests.size(); ++i)
		{
			auto& test = g_State->Tests[i];

			RunTestRecursive(test);
			if (test.ExpectedResult != ETestResult::NotRun)
			{
				if (test.ExpectedResult == test.Result.Result)
				{
					test.Result.Result = ETestResult::Success;
				}
				else
				{
					switch (test.Result.Result)
					{
					case ETestResult::Success:
					case ETestResult::Skip:
					case ETestResult::Fail: test.Result.Result = ETestResult::Fail; break;
					default: break;
					}
				}
			}
			OutputTestResult(test);
		}
	}
	// #endif

	bool SupportsCrashHandling()
	{
		return SUPPORT_SEPARATE_TEST_RUNNER;
	}

	uint64_t GetFlagsForArgs(size_t argc, const char* const* argv)
	{
		uint64_t flags = 0;
		for (size_t i = 0; i < argc; ++i)
		{
			std::string_view arg = argv[i];
			if (arg.starts_with("__int_test_runner="))
				flags |= c_IntTestRunner;
			else if (arg == "--compact")
				flags |= c_Compact;
			else if (arg == "--no-test-runner")
				flags |= c_NoTestRunner;
		}
		return flags;
	}

	void Begin(uint64_t flags)
	{
		g_State  = new State();
		*g_State = {
			.Flags = flags
		};
	}

	void HandleArgs(size_t argc, const char* const* argv)
	{
		if (!g_State)
			return;

		for (size_t i = 0; i < argc; ++i)
		{
			std::string_view arg = argv[i];
			if (arg.starts_with("__int_test_runner="))
			{
				uint64_t id        = std::strtoull(arg.substr(18).data(), nullptr, 10);
				g_State->IntTestID = id;
			}
			else if (arg.starts_with("--timed-max-duration="))
			{
				double timedMaxDuration   = std::strtod(arg.substr(22).data(), nullptr);
				g_State->TimedMaxDuration = timedMaxDuration;
			}
		}
	}

	void End()
	{
		if (!g_State)
			std::exit(1);

#if SUPPORT_SEPARATE_TEST_RUNNER
		if (g_State->Flags & c_NoTestRunner)
#endif
			RunTests();
#if SUPPORT_SEPARATE_TEST_RUNNER
		else if (g_State->Flags & c_IntTestRunner)
			RunTestRunner();
		else
			CreateTestRunner();
#endif
		OutputGroupChange(g_State->IntCurOutputGroup, ~size_t(0));

		delete g_State;

		std::exit(0);
	}

	void PushGroup(std::string name)
	{
		if (!g_State)
			return;

		g_State->Groups.emplace_back() = Group {
			.Name   = std::move(name),
			.Parent = g_State->GroupHierarchy.empty() ? ~size_t(0) : g_State->GroupHierarchy.back(),
		};
		g_State->GroupHierarchy.emplace_back(g_State->Groups.size() - 1);
		if (!g_State->IntFullGroupName.empty())
			g_State->IntFullGroupName += '.';
		g_State->IntFullGroupName += g_State->Groups.back().Name;
	}

	void PopGroup()
	{
		if (!g_State)
			return;

		size_t groupNameLen = g_State->Groups[g_State->GroupHierarchy.back()].Name.size();
		if (g_State->IntFullGroupName.size() != groupNameLen)
			++groupNameLen;
		g_State->IntFullGroupName.erase(g_State->IntFullGroupName.end() - groupNameLen, g_State->IntFullGroupName.end());
		g_State->GroupHierarchy.pop_back();
	}

	TestSpec& TestSpec::OnPreTest(TestFn&& onPreTest)
	{
		((TestState*) this)->OnPreTest = std::move(onPreTest);
		return *this;
	}

	TestSpec& TestSpec::OnTest(TestFn&& onTest, const std::source_location& loc)
	{
		((TestState*) this)->OnTest          = std::move(onTest);
		((TestState*) this)->Result.Location = std::format("{}:{}", loc.file_name(), loc.line());
		return *this;
	}

	TestSpec& TestSpec::OnPostTest(TestFn&& onPostTest)
	{
		((TestState*) this)->OnPostTest = std::move(onPostTest);
		return *this;
	}

	TestSpec& TestSpec::OnException(ExceptionHandlerFn&& onException)
	{
		((TestState*) this)->OnException = std::move(onException);
		return *this;
	}

	TestSpec& TestSpec::Dependencies(std::vector<std::string> dependencies)
	{
		((TestState*) this)->Dependencies = std::move(dependencies);
		return *this;
	}

	TestSpec& TestSpec::ExpectResult(ETestResult result)
	{
		((TestState*) this)->ExpectedResult = result;
		if (result == ETestResult::Crash)
			WillCrash();
		return *this;
	}

	TestSpec& TestSpec::Hide()
	{
		((TestState*) this)->Hidden = true;
		return *this;
	}

	TestSpec& TestSpec::WillCrash()
	{
		((TestState*) this)->WillCrash = true;
		return *this;
	}

	TestSpec& TestSpec::Time(double baselineTime)
	{
		((TestState*) this)->Timed        = true;
		((TestState*) this)->BaselineTime = baselineTime;
		return *this;
	}

	TestSpec& TestSpec::TimeUnit(std::string_view unit, size_t baseCount)
	{
		((TestState*) this)->Timed     = true;
		((TestState*) this)->TimeUnit  = unit;
		((TestState*) this)->BaseCount = baseCount;
		return *this;
	}

	TestSpec& Test(std::string name)
	{
		assert(g_State && "Testing::Test() must be called between Testing::Begin() and Testing::End()");
		if (!g_State)
			exit(0);

		auto& test = g_State->Tests.emplace_back();

		test = TestState {
			.Name  = std::move(name),
			.Group = g_State->GroupHierarchy.empty() ? ~size_t(0) : g_State->GroupHierarchy.back()
		};

		std::string fullName = g_State->IntFullGroupName;
		if (!fullName.empty())
			fullName += '.';
		fullName += test.Name;
		g_State->IntTestToID.insert({ std::move(fullName), g_State->Tests.size() - 1 });
		return *(TestSpec*) &test;
	}

	void Expect(bool expectation, const std::source_location& loc)
	{
		if (!expectation)
			Fail(loc);
	}

	void Success()
	{
		throw TestResult { .Result = ETestResult::Success };
	}

	void Skip(const std::source_location& loc)
	{
		throw TestResult {
			.Location = std::format("{}:{}", loc.file_name(), loc.line()),
			.Result   = ETestResult::Skip
		};
	}

	void Fail(const std::source_location& loc)
	{
		throw TestResult {
			.Location = std::format("{}:{}", loc.file_name(), loc.line()),
			.Result   = ETestResult::Fail
		};
	}
} // namespace Testing