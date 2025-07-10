#include "TestRunner.h"

#if SUPPORT_SEPARATE_TEST_RUNNER && BUILD_IS_SYSTEM_WINDOWS

	#include "State.h"

	#include <cassert>

	#include <format>
	#include <iostream>
	#include <random>
	#include <thread>

	#include <Windows.h>

namespace Testing
{
	void RunTest(TestState& test);
	void OutputTestResult(TestState& test);

	static std::string GetExeFilename()
	{
		std::string filename;
		filename.resize(1024);
		while (true)
		{
			DWORD size = GetModuleFileNameA(nullptr, filename.data(), (DWORD) filename.size());
			if (size < filename.size())
			{
				filename.resize(size);
				break;
			}

			switch (GetLastError())
			{
			case ERROR_INSUFFICIENT_BUFFER: filename.resize(filename.size() * 2); break;
			default: assert(false && "Failed to get exe filename");
			}
		}
		return filename;
	}

	static std::string GetCmdLine()
	{
		return GetCommandLineA();
	}

	static std::string GetCurrentDir()
	{
		std::string currentDirectory;
		currentDirectory.resize(1024);
		while (true)
		{
			DWORD size = GetCurrentDirectoryA((DWORD) currentDirectory.size(), currentDirectory.data());
			if (size < currentDirectory.size())
			{
				currentDirectory.resize(size);
				break;
			}

			switch (GetLastError())
			{
			case ERROR_INSUFFICIENT_BUFFER: currentDirectory.resize(currentDirectory.size() * 2); break;
			default: assert(false && "Failed to get current directory");
			}
		}
		return currentDirectory;
	}

	static constexpr uint32_t PacketCookie = 0xD9A8'1B26;
	struct Packet
	{
		uint32_t Cookie = PacketCookie;
		uint8_t  Type;
		uint8_t  Data1[3];
		uint64_t Data2;
	};

	static constexpr uint8_t HelloPacket  = 1;
	static constexpr uint8_t HiPacket     = 2;
	static constexpr uint8_t EndPacket    = 3;
	static constexpr uint8_t RunPacket    = 4;
	static constexpr uint8_t ResultPacket = 5;
	static constexpr uint8_t ReadyPacket  = 6;

	static bool RunTestRecursive(size_t testID, HANDLE hPipe, HANDLE hProc, OVERLAPPED* pOverlapped)
	{
		auto& test = g_State->Tests[testID];
		if (test.Result != ETestResult::NotRun)
			return true;

		bool skip = false;
		for (auto& dependency : test.Desc.Dependencies)
		{
			auto itr = g_State->IntTestToID.find(dependency);
			if (itr == g_State->IntTestToID.end())
			{
				// TODO: Output error
				skip = true;
				break;
			}

			RunTestRecursive(itr->second, hPipe, hProc, pOverlapped);
			skip = skip || (g_State->Tests[itr->second].Result != ETestResult::Success);
		}

		if (skip)
		{
			test.Result = test.Desc.ExpectSkip ? ETestResult::Success : ETestResult::Skip;
			return true;
		}
		if (test.Desc.ExpectSkip)
		{
			test.Result = ETestResult::Fail;
			return true;
		}

		Packet packet  = { .Type = RunPacket };
		packet.Data2   = testID;
		DWORD read     = 0;
		BOOL  fSuccess = WriteFile(hPipe, &packet, sizeof(packet), &read, nullptr);
		if (!fSuccess)
			return false;

		fSuccess = ReadFile(hPipe, &packet, sizeof(packet), &read, pOverlapped);
		if (!fSuccess && GetLastError() != ERROR_IO_PENDING)
			return false;

		HANDLE handles[2] { hProc, pOverlapped->hEvent };
		switch (WaitForMultipleObjects(2, handles, false, 60'000))
		{
		case WAIT_OBJECT_0:
			g_State->Tests[testID].Result = ETestResult::Crash;
			return false;
		case WAIT_OBJECT_0 + 1:
			ResetEvent(pOverlapped->hEvent);
			fSuccess = GetOverlappedResult(hPipe, pOverlapped, &read, false);
			if (!fSuccess || read != sizeof(Packet) || packet.Cookie != PacketCookie || packet.Type != ResultPacket)
			{
				g_State->Tests[testID].Result = test.Desc.ExpectCrash ? ETestResult::Success : ETestResult::Crash;
				return false;
			}

			g_State->Tests[testID].Result = (ETestResult) packet.Data1[0];

			fSuccess = ReadFile(hPipe, &packet, sizeof(packet), &read, nullptr);
			if (!fSuccess || read != sizeof(Packet) || packet.Cookie != PacketCookie || packet.Type != ReadyPacket)
				return false;
			break;
		case WAIT_TIMEOUT:
			g_State->Tests[testID].Result = ETestResult::TimedOut;
			return false;
		}
		return true;
	}

	void CreateTestRunner()
	{
		std::string filename         = GetExeFilename();
		std::string origCommandLine  = GetCmdLine();
		std::string currentDirectory = GetCurrentDir();

		std::mt19937_64 rng(std::random_device {}());

		size_t currentTest = 0;
		size_t retryCount  = 0;
		while (currentTest < g_State->Tests.size() && retryCount < 4)
		{
			HANDLE     namedPipe = INVALID_HANDLE_VALUE;
			OVERLAPPED overlapped {};
			overlapped.hEvent = CreateEventA(nullptr, true, false, nullptr);

			std::atomic_bool running = false;

			uint64_t id = 0;
			while (id == 0 && (namedPipe == INVALID_HANDLE_VALUE || namedPipe == 0))
			{
				id            = rng();
				auto pipePath = std::format("\\\\.\\pipe\\common_build_testrunner_{}", id);
				namedPipe     = CreateNamedPipeA(
                    pipePath.c_str(),
                    PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED,
                    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
                    1,
                    4096,
                    4096,
                    0,
                    nullptr);
			}
			assert(namedPipe != 0 && "BUG!!!");

			std::string commandLine = origCommandLine;
			commandLine            += std::format(" __int_test_runner={}", id);

			HANDLE hNulFile = CreateFileA("NUL", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

			STARTUPINFOA startupInfo {
				.cb         = sizeof(startupInfo),
				.dwFlags    = STARTF_USESTDHANDLES,
				.hStdInput  = hNulFile,
				.hStdOutput = hNulFile,
				.hStdError  = hNulFile
			};
			PROCESS_INFORMATION procInfo {};
			std::atomic_bool    processCreated = false;

			std::jthread dbgThread = std::jthread([&]() {
				processCreated = CreateProcessA(filename.c_str(), commandLine.data(), nullptr, nullptr, true, DEBUG_ONLY_THIS_PROCESS, nullptr, currentDirectory.c_str(), &startupInfo, &procInfo) != 0;
				processCreated.notify_all();

				DEBUG_EVENT debugEvent {};
				while (WaitForDebugEvent(&debugEvent, INFINITE))
				{
					if (debugEvent.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
					{
						ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
						break;
					}
					if (debugEvent.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
					{
						switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode)
						{
						case EXCEPTION_ACCESS_VIOLATION:
						case EXCEPTION_DATATYPE_MISALIGNMENT:
							ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
							TerminateProcess(procInfo.hProcess, (DWORD) -1);
							break;
						default:
							ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
							break;
						}
					}
					else
					{
						ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
					}
				}
			});
			processCreated.wait(false);
			assert(processCreated && "Failed to create test runner process");

			assert(ConnectNamedPipe(namedPipe, nullptr) != 0 && "Failed to connect to test runner process");

			Packet packet {};
			DWORD  read     = 0;
			BOOL   fSuccess = ReadFile(namedPipe, &packet, sizeof(packet), &read, nullptr);
			if (!fSuccess || read != sizeof(Packet) || packet.Cookie != PacketCookie || packet.Type != HelloPacket)
			{
				++retryCount;
				CloseHandle(overlapped.hEvent);
				CloseHandle(namedPipe);
				CloseHandle(procInfo.hThread);
				CloseHandle(procInfo.hProcess);
				continue;
			}

			packet   = { .Type = HiPacket };
			fSuccess = WriteFile(namedPipe, &packet, sizeof(packet), &read, nullptr);
			if (!fSuccess)
			{
				++retryCount;
				CloseHandle(overlapped.hEvent);
				CloseHandle(namedPipe);
				CloseHandle(procInfo.hThread);
				CloseHandle(procInfo.hProcess);
				continue;
			}

			while (currentTest < g_State->Tests.size())
			{
				bool cont = RunTestRecursive(currentTest, namedPipe, procInfo.hProcess, &overlapped);
				OutputTestResult(g_State->Tests[currentTest]);
				++currentTest;
				if (!cont)
					break;
			}

			packet   = { .Type = EndPacket };
			fSuccess = WriteFile(namedPipe, &packet, sizeof(packet), &read, nullptr);
			if (!fSuccess)
			{
				CloseHandle(overlapped.hEvent);
				CloseHandle(namedPipe);
				CloseHandle(procInfo.hThread);
				CloseHandle(procInfo.hProcess);
				continue;
			}

			CloseHandle(overlapped.hEvent);
			CloseHandle(namedPipe);
			CloseHandle(procInfo.hThread);
			CloseHandle(procInfo.hProcess);
		}
	}

	void RunTestRunner()
	{
		_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

		auto   pipePath = std::format("\\\\.\\pipe\\common_build_testrunner_{}", g_State->IntTestID);
		HANDLE hPipe    = CreateFileA(pipePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
		assert(hPipe != INVALID_HANDLE_VALUE && "Failed to connect to test runner");

		DWORD dwMode = PIPE_READMODE_MESSAGE;
		assert(SetNamedPipeHandleState(hPipe, &dwMode, nullptr, nullptr) != 0 && "Failed to setup pipe");

		Packet packet { .Type = HelloPacket };
		DWORD  read     = 0;
		BOOL   fSuccess = WriteFile(hPipe, &packet, sizeof(packet), &read, nullptr);
		if (!fSuccess)
		{
			CloseHandle(hPipe);
			return;
		}

		fSuccess = ReadFile(hPipe, &packet, sizeof(packet), &read, nullptr);
		if (!fSuccess || read != sizeof(Packet) || packet.Cookie != PacketCookie || packet.Type != HiPacket)
		{
			CloseHandle(hPipe);
			return;
		}

		while (true)
		{
			fSuccess = ReadFile(hPipe, &packet, sizeof(packet), &read, nullptr);
			if (!fSuccess || read != sizeof(Packet) || packet.Cookie != PacketCookie)
				break;

			switch (packet.Type)
			{
			case HelloPacket:
			case HiPacket: continue;
			case EndPacket: break;
			case RunPacket:
			{
				uint64_t test = packet.Data2;
				if (test < g_State->Tests.size())
					RunTest(g_State->Tests[test]);
				packet          = { .Type = ResultPacket };
				packet.Data1[0] = (uint8_t) (test < g_State->Tests.size() ? g_State->Tests[test].Result : ETestResult::NotRun);
				fSuccess        = WriteFile(hPipe, &packet, sizeof(packet), &read, nullptr);
				if (!fSuccess)
					break;
				if (test < g_State->Tests.size() && g_State->Tests[test].Desc.OnPostTest)
					g_State->Tests[test].Desc.OnPostTest();

				packet   = { .Type = ReadyPacket };
				fSuccess = WriteFile(hPipe, &packet, sizeof(packet), &read, nullptr);
				if (!fSuccess)
					break;
				break;
			}
			}

			if (!fSuccess)
				break;
		}

		CloseHandle(hPipe);
	}
} // namespace Testing
#endif