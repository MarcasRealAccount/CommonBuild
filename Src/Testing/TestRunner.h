#pragma once

#include "Build.h"

#if BUILD_IS_SYSTEM_WINDOWS
	#define SUPPORT_SEPARATE_TEST_RUNNER 0
#else
	#define SUPPORT_SEPARATE_TEST_RUNNER 0
#endif

#if SUPPORT_SEPARATE_TEST_RUNNER

namespace Testing
{
	void CreateTestRunner();
	void RunTestRunner();
} // namespace Testing

#endif