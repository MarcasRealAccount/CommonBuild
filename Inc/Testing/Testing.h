#pragma once

#include <string_view>

namespace Testing
{
	using TestFunc      = void (*)();
	using TestOnEndFunc = void (*)();

	void Begin();
	void End();

	void PushGroup(std::string_view name);
	void PopGroup();

	void Test(std::string_view name, TestFunc test, TestOnEndFunc onEnd = nullptr);
	void TimedTest(std::string_view name, double baseline, TestFunc test, TestOnEndFunc onEnd = nullptr);

	void Assert(bool statement);
	void Fail();
	void Skip();
} // namespace Testing