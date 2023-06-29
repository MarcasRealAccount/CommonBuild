#include "Testing/Testing.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace Testing
{
	std::size_t s_TestCount    = 0;
	std::size_t s_TestsSuccess = 0;

	std::string_view s_GroupName;
	std::size_t      s_GroupTestCount    = 0;
	std::size_t      s_GroupTestsSuccess = 0;

	void Begin()
	{
		s_TestCount    = 0;
		s_TestsSuccess = 0;
	}

	void End()
	{
		std::ostringstream str;
		str << "--- (";
		if (s_TestsSuccess < s_TestCount)
		{
			float factor = static_cast<float>(s_TestsSuccess) / static_cast<float>(s_TestCount);
			if (factor < 0.5)
				str << "\033[31m";
			else
				str << "\033[33m";
		}
		else
		{
			str << "\033[32m";
		}
		str << s_TestsSuccess << "/" << s_TestCount << "\033[39m) ---\n";
		std::cout << str.str();
	}

	void PushGroup(std::string_view name)
	{
		s_GroupName         = name;
		s_GroupTestCount    = 0;
		s_GroupTestsSuccess = 0;
		std::ostringstream str;
		str << "--- " << name << " ---\n";
		std::cout << str.str();
	}

	void PopGroup()
	{
		std::ostringstream str;
		str << "--- (";
		if (s_GroupTestsSuccess < s_GroupTestCount)
		{
			float factor = static_cast<float>(s_GroupTestsSuccess) / static_cast<float>(s_GroupTestCount);
			if (factor < 0.5)
				str << "\033[31m";
			else
				str << "\033[33m";
		}
		else
		{
			str << "\033[32m";
		}
		str << s_GroupTestsSuccess << "/" << s_GroupTestCount << "\033[39m) ---\n";
		std::cout << str.str();
	}

	enum class ETestResult
	{
		Success,
		Fail,
		Skip
	};

	void Test(std::string_view name, TestFunc test, TestOnEndFunc onEnd)
	{
		++s_TestCount;
		++s_GroupTestCount;

		ETestResult result = ETestResult::Success;

		try
		{
			test();
		}
		catch (ETestResult res)
		{
			result = res;
		}
		catch (...)
		{
			result = ETestResult::Fail;
		}

		std::ostringstream str;
		switch (result)
		{
		case ETestResult::Success:
			++s_GroupTestsSuccess;
			++s_TestsSuccess;
			str << "\033[32mSUCCESS: \033[39m" << name << '\n';
			break;
		case ETestResult::Fail:
			str << "\033[31mFAIL:    \033[39m" << name << '\n';
			break;
		case ETestResult::Skip:
			str << "\033[33mSKIP:    \033[39m" << name << '\n';
			break;
		}
		std::cout << str.str();

		if (onEnd)
			onEnd();
	}

	void TimedTest(std::string_view name, double baseline, TestFunc test, TestOnEndFunc onEnd)
	{
		using Clock = std::chrono::high_resolution_clock;

		++s_TestCount;
		++s_GroupTestCount;

		ETestResult result = ETestResult::Success;
		double      time   = 0.0;
		std::size_t iters  = 0;

		try
		{
			auto start = Clock::now();
			while (true)
			{
				using namespace std::chrono_literals;
				auto cur = Clock::now();
				if ((cur - start) >= 100ms)
					break;
				test();
				++iters;
			}
			auto end = Clock::now();
			time     = std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(end - start).count() / iters;
		}
		catch (ETestResult res)
		{
			result = res;
		}
		catch (...)
		{
			result = ETestResult::Fail;
		}

		std::ostringstream str;
		switch (result)
		{
		case ETestResult::Success:
		{
			double factor = baseline / time;
			if (factor < 0.5)
			{
				str << "\033[31m";
			}
			else if (factor < 0.75)
			{
				str << "\033[33m";
			}
			else
			{
				++s_GroupTestsSuccess;
				++s_TestsSuccess;
				str << "\033[32m";
			}
			str << std::scientific << std::setprecision(3) << time << '/' << std::scientific << std::setprecision(3) << baseline << ": \033[39m" << name << '\n';
			break;
		}
		case ETestResult::Fail:
			str << "\033[31mFAIL:    \033[39m" << name << '\n';
			break;
		case ETestResult::Skip:
			str << "\033[33mSKIP:    \033[39m" << name << '\n';
			break;
		}
		std::cout << str.str();

		if (onEnd)
			onEnd();
	}

	double TimedBasline(TestFunc test)
	{
		using Clock       = std::chrono::high_resolution_clock;
		std::size_t iters = 0;
		auto        start = Clock::now();
		while (true)
		{
			using namespace std::chrono_literals;
			auto cur = Clock::now();
			if ((cur - start) >= 100ms)
				break;
			test();
			++iters;
		}
		auto end = Clock::now();
		return std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(end - start).count() / iters;
	}

	void Assert(bool statement)
	{
		if (!statement)
			Fail();
	}

	void Fail()
	{
		throw ETestResult::Fail;
	}

	void Skip()
	{
		throw ETestResult::Skip;
	}
} // namespace Testing