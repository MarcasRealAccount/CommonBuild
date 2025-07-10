#include "Testing/Testing.h"
#include "State.h"
#include "TestRunner.h"

#include <cassert>
#include <cwchar>

#include <format>
#include <iostream>

namespace Testing
{
	State* g_State = nullptr;

	void RunTest(TestState& test)
	{
		if (test.Result != ETestResult::NotRun)
			return;

#if !SUPPORT_SEPARATE_TEST_RUNNER
		if (test.Desc.ExpectCrash || test.Desc.WillCrash)
		{
			test.Result = ETestResult::Fail;
			return;
		}
#endif

		try
		{
			test.Desc.OnTest();
			test.Result = ETestResult::Success;
		}
		catch (ETestResult result)
		{
			test.Result = result;
		}
		catch (...)
		{
			test.Result = test.Desc.OnException() ? ETestResult::Success : ETestResult::Fail;
		}
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

	void OutputTestResult(TestState& test)
	{
		if (test.Desc.Hidden)
			return;

		if (test.Result == ETestResult::Success)
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
		switch (test.Result)
		{
		case ETestResult::Success: resultStr = "  \033[32mSuccess"; break;
		case ETestResult::Skip: resultStr = "     \033[33mSkip"; break;
		case ETestResult::Fail: resultStr = "     \033[31mFail"; break;
		case ETestResult::Crash: resultStr = "    \033[31mCrash"; break;
		case ETestResult::TimedOut: resultStr = "\033[34mTimed Out"; break;
		default: resultStr = "     \033[31mFAIL"; break;
		}
		std::cout << std::format("{:{}}{}\033[39m: {}\n", "", 2 * g_State->IntCurGroupDepth, resultStr, test.Name);
	}

	static void RunTestRecursive(TestState& test)
	{
		if (test.Result != ETestResult::NotRun)
			return;

		bool skip = false;
		for (auto& dependency : test.Desc.Dependencies)
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
			skip = skip || (depTest.Result != ETestResult::Success);
		}

		if (skip)
		{
			test.Result = test.Desc.ExpectSkip ? ETestResult::Success : ETestResult::Skip;
			return;
		}
		if (test.Desc.ExpectSkip)
		{
			test.Result = ETestResult::Fail;
			return;
		}
		RunTest(test);
	}

#if !SUPPORT_SEPARATE_TEST_RUNNER
	static void RunTests()
	{
		for (auto failed : g_State->IntTestsFailed)
			g_State->Tests[failed].Result = ETestResult::Fail;

		for (size_t i = g_State->IntTestsStart; i < g_State->Tests.size(); ++i)
		{
			auto& test = g_State->Tests[i];

			RunTestRecursive(test);
			OutputTestResult(test);
		}
	}
#endif

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
		}
	}

	void End()
	{
		if (!g_State)
			std::exit(1);

#if SUPPORT_SEPARATE_TEST_RUNNER
		if (g_State->Flags & c_IntTestRunner)
			RunTestRunner();
		else
			CreateTestRunner();
#else
		RunTests();
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

	void Test(std::string name, TestDesc desc)
	{
		if (!g_State)
			return;

		g_State->Tests.emplace_back() = TestState {
			.Name   = std::move(name),
			.Group  = g_State->GroupHierarchy.empty() ? ~size_t(0) : g_State->GroupHierarchy.back(),
			.Desc   = std::move(desc),
			.Result = ETestResult::NotRun
		};
		if (!g_State->Tests.back().Desc.Hidden)
		{
			for (auto group : g_State->GroupHierarchy)
				++g_State->Groups[group].Total;
		}

		std::string fullName = g_State->IntFullGroupName;
		if (!fullName.empty())
			fullName += '.';
		fullName += g_State->Tests.back().Name;
		g_State->IntTestToID.insert({ std::move(fullName), g_State->Tests.size() - 1 });
	}

	void Expect(bool expectation)
	{
		if (!expectation)
			Fail();
	}

	void Success()
	{
		throw ETestResult::Success;
	}

	void Skip()
	{
		throw ETestResult::Skip;
	}

	void Fail()
	{
		throw ETestResult::Fail;
	}
} // namespace Testing