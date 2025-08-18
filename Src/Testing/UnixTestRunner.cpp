#include "Testing/Testing.h"
#include "TestRunner.h"

#if SUPPORT_SEPARATE_TEST_RUNNER && BUILD_IS_SYSTEM_UNIX

	#include "State.h"

	#include <assert.h>
	#include <signal.h>

	#include <string>

	#include <fcntl.h>
	#include <sys/socket.h>
	#include <sys/un.h>
	#include <sys/wait.h>
	#include <unistd.h>

	#if BUILD_IS_SYSTEM_MACOSX
		#include <mach-o/dyld.h>
	#endif

namespace Testing
{
	void RunTest(TestState& test);
	void RunTimedTest(TestState& test);
	void OutputTestResult(TestState& test);

	static std::string GetExeFilename()
	{
		char    buffer[1024];
		ssize_t len = ::readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
		if (len == -1)
		{
	#if BUILD_IS_SYSTEM_MACOSX
			uint32_t size = sizeof(buffer);
			if (_NSGetExecutablePath(buffer, &size) == 0)
				return std::string(buffer);
			else
				assert(false && "Failed to get exe filename");
	#else
			assert(false && "Failed to get exe filename");
	#endif
		}
		buffer[len] = '\0';
		return std::string(buffer);
	}

	static std::string GetCmdLine()
	{
		std::string cmd;
		FILE*       f = fopen("/proc/self/cmdline", "r");
		if (f)
		{
			char c;
			while (fread(&c, 1, 1, f))
			{
				if (c == '\0')
					cmd.push_back(' ');
				else
					cmd.push_back(c);
			}
			fclose(f);
		}
		return cmd;
	}

	static std::string GetCurrentDir()
	{
		char buffer[1024];
		if (::getcwd(buffer, sizeof(buffer)) == nullptr)
			assert(false && "Failed to get current directory");
		return std::string(buffer);
	}

	static constexpr uint32_t PacketCookie = 0xD9A81B26;
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

	static bool WritePacket(int fd, const Packet& packet)
	{
		ssize_t written = ::write(fd, &packet, sizeof(packet));
		return written == sizeof(packet);
	}

	static bool ReadPacket(int fd, Packet& packet)
	{
		ssize_t r = ::read(fd, &packet, sizeof(packet));
		return r == sizeof(Packet) && packet.Cookie == PacketCookie;
	}

	static bool RunTestRecursive(size_t testID, int sockfd, pid_t child)
	{
		auto& test = g_State->Tests[testID];
		if (test.Result != ETestResult::NotRun)
			return true;

		bool skip = false;
		for (auto& dependency : test.Dependencies)
		{
			auto itr = g_State->IntTestToID.find(dependency);
			if (itr == g_State->IntTestToID.end())
			{
				skip = true;
				break;
			}

			RunTestRecursive(itr->second, sockfd, child);
			if (g_State->Tests[itr->second].ExpectedResult == g_State->Tests[itr->second].Result)
				g_State->Tests[itr->second].Result = ETestResult::Success;
			skip = skip || (g_State->Tests[itr->second].Result != ETestResult::Success);
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
			test.Result = ETestResult::Skip;
			return true;
		}

		Packet packet { .Type = RunPacket };
		packet.Data2 = testID;
		if (!WritePacket(sockfd, packet))
			return false;

		if (!ReadPacket(sockfd, packet) || packet.Type != ResultPacket)
		{
			g_State->Tests[testID].Result = ETestResult::Crash;
			return false;
		}

		g_State->Tests[testID].Result = (ETestResult) packet.Data1[0];
		g_State->Tests[testID].Time   = *(double*) &packet.Data2;

		if (!ReadPacket(sockfd, packet) || packet.Type != ReadyPacket)
			return false;

		return true;
	}

	void CreateTestRunner()
	{
		std::string filename         = GetExeFilename();
		std::string origCommandLine  = GetCmdLine();
		std::string currentDirectory = GetCurrentDir();

		signal(SIGPIPE, SIG_IGN);

		size_t currentTest = 0;
		size_t retryCount  = 0;
		while (currentTest < g_State->Tests.size() && retryCount < 4)
		{
			int sockets[2];
			if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0)
			{
				assert(false && "Failed to create socketpair");
				return;
			}

			pid_t child = fork();
			if (child == -1)
			{
				::close(sockets[0]);
				::close(sockets[1]);
				assert(false && "Failed to fork");
				return;
			}

			if (child == 0)
			{
				/*int devnull = ::open("/dev/null", O_WRONLY);
				if (devnull >= 0)
				{
					::dup2(devnull, STDERR_FILENO);
					::close(devnull);
				}*/

				::dup2(sockets[1], STDIN_FILENO);
				::dup2(sockets[1], STDOUT_FILENO);
				::close(sockets[0]);
				::close(sockets[1]);

				execl(filename.c_str(), filename.c_str(), "__int_test_runner=2327489", nullptr);
				_exit(1);
			}

			::close(sockets[1]);
			int sockfd = sockets[0];

			Packet packet;
			if (!ReadPacket(sockfd, packet) || packet.Type != HelloPacket)
			{
				++retryCount;
				::close(sockfd);
				continue;
			}

			packet = { .Type = HiPacket };
			if (!WritePacket(sockfd, packet))
			{
				++retryCount;
				::close(sockfd);
				continue;
			}

			while (currentTest < g_State->Tests.size())
			{
				bool  cont = RunTestRecursive(currentTest, sockfd, child);
				auto& test = g_State->Tests[currentTest];
				if (test.ExpectedResult != ETestResult::NotRun)
				{
					if (test.ExpectedResult == test.Result)
					{
						test.Result = ETestResult::Success;
					}
					else
					{
						switch (test.Result)
						{
						case ETestResult::Success:
						case ETestResult::Skip:
						case ETestResult::Fail: test.Result = ETestResult::Fail; break;
						default: break;
						}
					}
				}
				OutputTestResult(test);
				++currentTest;
				if (!cont)
					break;
			}

			packet = { .Type = EndPacket };
			WritePacket(sockfd, packet);
			::close(sockfd);

			int status = 0;
			waitpid(child, &status, 0);
		}
	}

	void RunTestRunner()
	{
		Packet packet { .Type = HelloPacket };
		if (!WritePacket(STDOUT_FILENO, packet))
			return;

		if (!ReadPacket(STDIN_FILENO, packet) || packet.Type != HiPacket)
			return;

		while (true)
		{
			if (!ReadPacket(STDIN_FILENO, packet))
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
				{
					RunTest(g_State->Tests[test]);
					if (g_State->Tests[test].Timed && g_State->Tests[test].Result == ETestResult::Success)
						RunTimedTest(g_State->Tests[test]);
				}
				packet          = { .Type = ResultPacket };
				packet.Data1[0] = (uint8_t) (test < g_State->Tests.size() ? g_State->Tests[test].Result : ETestResult::NotRun);
				packet.Data2    = test < g_State->Tests.size() ? *(uint64_t*) &g_State->Tests[test].Time : 0;
				if (!WritePacket(STDOUT_FILENO, packet))
					break;
				if (test < g_State->Tests.size() && g_State->Tests[test].OnPostTest)
					g_State->Tests[test].OnPostTest();

				packet = { .Type = ReadyPacket };
				if (!WritePacket(STDOUT_FILENO, packet))
					break;
				break;
			}
			}
		}
	}
} // namespace Testing

#endif