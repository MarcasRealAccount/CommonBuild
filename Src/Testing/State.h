#pragma once

#include "Testing/Testing.h"

#include <cstddef>
#include <cstdint>

#include <string>
#include <unordered_map>
#include <vector>

namespace Testing
{
	enum class ETestResult : uint8_t
	{
		NotRun,
		Success,
		Skip,
		Fail,
		Crash,
		TimedOut
	};

	struct TestState
	{
		std::string Name;
		size_t      Group = 0;
		TestDesc    Desc;
		ETestResult Result = ETestResult::NotRun;
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