#pragma once

#include "Common/Flags.h"

#define BUILD_CONFIG_UNKNOWN 0
#define BUILD_CONFIG_DEBUG   1
#define BUILD_CONFIG_RELEASE 2
#define BUILD_CONFIG_DIST    3

#define BUILD_SYSTEM_UNKNOWN 0
#define BUILD_SYSTEM_WINDOWS 1
#define BUILD_SYSTEM_XBOX    2
#define BUILD_SYSTEM_MACOSX  3
#define BUILD_SYSTEM_IOS     4
#define BUILD_SYSTEM_LINUX   5
#define BUILD_SYSTEM_ANDROID 6

#define BUILD_TOOLSET_UNKNOWN 0
#define BUILD_TOOLSET_MSVC    1
#define BUILD_TOOLSET_CLANG   2
#define BUILD_TOOLSET_GCC     3

#define BUILD_PLATFORM_UNKNOWN 0
#define BUILD_PLATFORM_X86     1
#define BUILD_PLATFORM_AMD64   2
#define BUILD_PLATFORM_ARM32   3
#define BUILD_PLATFORM_ARM64   4

#define BUILD_IS_CONFIG_DEBUG ((BUILD_CONFIG == BUILD_CONFIG_DEBUG) || (BUILD_CONFIG == BUILD_CONFIG_RELEASE))
#define BUILD_IS_CONFIG_DIST  ((BUILD_CONFIG == BUILD_CONFIG_RELEASE) || (BUILD_CONFIG == BUILD_CONFIG_DIST))

#define BUILD_IS_SYSTEM_UNIX      ((BUILD_SYSTEM == BUILD_SYSTEM_MACOSX) || (BUILD_SYSTEM == BUILD_SYSTEM_IOS) || (BUILD_SYSTEM == BUILD_SYSTEM_LINUX) || (BUILD_SYSTEM == BUILD_SYSTEM_ANDROID))
#define BUILD_IS_SYSTEM_APPLE     ((BUILD_SYSTEM == BUILD_SYSTEM_MACOSX) || (BUILD_SYSTEM == BUILD_SYSTEM_IOS))
#define BUILD_IS_SYSTEM_MICROSOFT (BUILD_SYSTEM == BUILD_SYSTEM_WINDOWS)
#define BUILD_IS_SYSTEM_WINDOWS   (BUILD_SYSTEM == BUILD_SYSTEM_WINDOWS)
#define BUILD_IS_SYSTEM_MACOSX    (BUILD_SYSTEM == BUILD_SYSTEM_MACOSX)
#define BUILD_IS_SYSTEM_IOS       (BUILD_SYSTEM == BUILD_SYSTEM_IOS)
#define BUILD_IS_SYSTEM_LINUX     (BUILD_SYSTEM == BUILD_SYSTEM_LINUX)
#define BUILD_IS_SYSTEM_ANDROID   (BUILD_SYSTEM == BUILD_SYSTEM_ANDROID)

#define BUILD_IS_TOOLSET_MSVC  (BUILD_TOOLSET == BUILD_TOOLSET_MSVC)
#define BUILD_IS_TOOLSET_CLANG (BUILD_TOOLSET == BUILD_TOOLSET_CLANG)
#define BUILD_IS_TOOLSET_GCC   (BUILD_TOOLSET == BUILD_TOOLSET_GCC)

#define BUILD_IS_PLATFORM_X86   (BUILD_PLATFORM == BUILD_PLATFORM_X86)
#define BUILD_IS_PLATFORM_AMD64 (BUILD_PLATFORM == BUILD_PLATFORM_AMD64)
#define BUILD_IS_PLATFORM_ARM32 (BUILD_PLATFORM == BUILD_PLATFORM_ARM32)
#define BUILD_IS_PLATFORM_ARM64 (BUILD_PLATFORM == BUILD_PLATFORM_ARM64)

namespace Common
{
	using BuildConfig   = Flags<>;
	using BuildSystem   = Flags<>;
	using BuildToolset  = Flags<>;
	using BuildPlatform = Flags<>;

	namespace BuildConfigs
	{
		static constexpr BuildConfig Unknown = 0;
		static constexpr BuildConfig Debug   = 1;
		static constexpr BuildConfig Dist    = 2;
	} // namespace BuildConfigs

	namespace BuildSystems
	{
		static constexpr BuildSystem Unknown   = 0x000;
		static constexpr BuildSystem Microsoft = 0x001;
		static constexpr BuildSystem Apple     = 0x002;
		static constexpr BuildSystem Unix      = 0x004;
		static constexpr BuildSystem Windows   = 0x008;
		static constexpr BuildSystem XBox      = 0x010;
		static constexpr BuildSystem MacOSX    = 0x020;
		static constexpr BuildSystem IOS       = 0x040;
		static constexpr BuildSystem Linux     = 0x080;
		static constexpr BuildSystem Android   = 0x100;
	} // namespace BuildSystems

	namespace BuildToolsets
	{
		static constexpr BuildToolset Unknown = 0;
		static constexpr BuildToolset MSVC    = 1;
		static constexpr BuildToolset Clang   = 2;
		static constexpr BuildToolset GCC     = 4;
	} // namespace BuildToolsets

	namespace BuildPlatforms
	{
		static constexpr BuildPlatform Unknown = 0;
		static constexpr BuildPlatform X86     = 1;
		static constexpr BuildPlatform AMD64   = 2;
		static constexpr BuildPlatform ARM32   = 4;
		static constexpr BuildPlatform ARM64   = 8;
	} // namespace BuildPlatforms

	constexpr BuildConfig GetBuildConfig()
	{
#if BUILD_CONFIG == BUILD_CONFIG_DEBUG
		return BuildConfigs::Debug;
#elif BUILD_CONFIG == BUILD_CONFIG_RELEASE
		return BuildConfigs::Debug | BuildConfigs::Dist;
#elif BUILD_CONFIG == BUILD_CONFIG_DIST
		return BuildConfigs::Dist;
#else
		return BuildConfigs::Unknown;
#endif
	}

	constexpr BuildSystem GetBuildSystem()
	{
#if BUILD_SYSTEM == BUILD_SYSTEM_WINDOWS
		return BuildSystems::Microsoft | BuildSystems::Windows;
#elif BUILD_SYSTEM == BUILD_SYSTEM_XBOX
		return BuildSystems::Microsoft | BuildSystems::XBox;
#elif BUILD_SYSTEM == BUILD_SYSTEM_MACOSX
		return BuildSystems::Apple | BuildSystems::Unix | BuildSystems::MacOSX;
#elif BUILD_SYSTEM == BUILD_SYSTEM_IOS
		return BuildSystems::Apple | BuildSystems::Unix | BuildSystems::IOS;
#elif BUILD_SYSTEM == BUILD_SYSTEM_LINUX
		return BuildSystems::Unix | BuildSystems::Linux;
#elif BUILD_SYSTEM == BUILD_SYSTEM_ANDROID
		return BuildSystems::Unix | BuildSystems::Android;
#else
		return BuildSystems::Unknown;
#endif
	}

	constexpr BuildToolset GetBuildToolset()
	{
#if BUILD_TOOLSET == BUILD_TOOLSET_MSVC
		return BuildToolsets::MSVC;
#elif BUILD_TOOLSET == BUILD_TOOLSET_CLANG
		return BuildToolsets::Clang;
#elif BUILD_TOOLSET == BUILD_TOOLSET_GCC
		return BuildToolsets::GCC;
#else
		return BuildToolsets::Unknown;
#endif
	}

	constexpr BuildPlatform GetBuildPlatform()
	{
#if BUILD_PLATFORM == BUILD_PLATFORM_X86
		return BuildPlatforms::X86;
#elif BUILD_PLATFORM == BUILD_PLATFORM_AMD64
		return BuildPlatforms::AMD64;
#elif BUILD_PLATFORM == BUILD_PLATFORM_ARM32
		return BuildPlatforms::ARM32;
#elif BUILD_PLATFORM == BUILD_PLATFORM_ARM64
		return BuildPlatforms::ARM64;
#else
		return BuildPlatforms::Unknown;
#endif
	}

	static constexpr BuildConfig   c_Config   = GetBuildConfig();
	static constexpr BuildSystem   c_System   = GetBuildSystem();
	static constexpr BuildToolset  c_Toolset  = GetBuildToolset();
	static constexpr BuildPlatform c_Platform = GetBuildPlatform();

	static constexpr bool c_IsConfigDebug = c_Config.HasFlag(BuildConfigs::Debug);
	static constexpr bool c_IsConfigDist  = c_Config.HasFlag(BuildConfigs::Dist);

	static constexpr bool c_IsSystemMicrosoft = c_System.HasFlag(BuildSystems::Microsoft);
	static constexpr bool c_IsSystemApple     = c_System.HasFlag(BuildSystems::Apple);
	static constexpr bool c_IsSystemUnix      = c_System.HasFlag(BuildSystems::Unix);
	static constexpr bool c_IsSystemWindows   = c_System.HasFlag(BuildSystems::Windows);
	static constexpr bool c_IsSystemXBox      = c_System.HasFlag(BuildSystems::XBox);
	static constexpr bool c_IsSystemMacOSX    = c_System.HasFlag(BuildSystems::MacOSX);
	static constexpr bool c_IsSystemIOS       = c_System.HasFlag(BuildSystems::IOS);
	static constexpr bool c_IsSystemLinux     = c_System.HasFlag(BuildSystems::Linux);
	static constexpr bool c_IsSystemAndroid   = c_System.HasFlag(BuildSystems::Android);

	static constexpr bool c_IsToolsetMSVC  = c_Toolset.HasFlag(BuildToolsets::MSVC);
	static constexpr bool c_IsToolsetClang = c_Toolset.HasFlag(BuildToolsets::Clang);
	static constexpr bool c_IsToolsetGCC   = c_Toolset.HasFlag(BuildToolsets::GCC);

	static constexpr bool c_IsPlatformX86   = c_Platform.HasFlag(BuildPlatforms::X86);
	static constexpr bool c_IsPlatformAMD64 = c_Platform.HasFlag(BuildPlatforms::AMD64);
	static constexpr bool c_IsPlatformARM32 = c_Platform.HasFlag(BuildPlatforms::ARM32);
	static constexpr bool c_IsPlatformARM64 = c_Platform.HasFlag(BuildPlatforms::ARM64);
} // namespace Common
