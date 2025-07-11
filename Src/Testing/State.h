#pragma once

#include "Testing/Testing.h"

#include <cstddef>
#include <cstdint>

#include <string>
#include <unordered_map>
#include <vector>

namespace Testing
{
	struct TestState
	{
		std::string Name;
		size_t      Group  = ~size_t(0);
		ETestResult Result = ETestResult::NotRun;

		TestFn             OnPreTest   = nullptr;
		TestFn             OnTest      = nullptr;
		TestFn             OnPostTest  = nullptr;
		ExceptionHandlerFn OnException = nullptr;

		std::vector<std::string> Dependencies;

		ETestResult ExpectedResult = ETestResult::Success;
		bool        Hidden         = false;
		bool        WillCrash      = false;
	};

	struct Group
	{
		std::string Name;
		size_t      Parent = 0;

		size_t Total   = 0;
		size_t Success = 0;
	};

	struct State
	{
		uint64_t Flags = 0;

		std::vector<Group>     Groups;
		std::vector<TestState> Tests;
		std::vector<size_t>    GroupHierarchy;

		uint64_t IntTestID = 0;

		size_t              IntTestsStart = 0;
		std::vector<size_t> IntTestsFailed;

		std::unordered_map<std::string, size_t> IntTestToID;
		std::string                             IntFullGroupName;

		size_t IntCurOutputGroup = ~size_t(0);
		size_t IntCurGroupDepth  = 0;
	};

	extern State* g_State;
} // namespace Testing