#pragma once

#include "Flags.h"

#define BUILD_CONFIG_UNKNOWN 0
#define BUILD_CONFIG_DEBUG   1
#define BUILD_CONFIG_RELEASE 2
#define BUILD_CONFIG_DIST    3

#define BUILD_SYSTEM_UNKNOWN 0
#define BUILD_SYSTEM_WINDOWS 1
#define BUILD_SYSTEM_MACOSX  2
#define BUILD_SYSTEM_LINUX   3

#define BUILD_TOOLSET_UNKNOWN 0
#define BUILD_TOOLSET_MSVC    1
#define BUILD_TOOLSET_CLANG   2
#define BUILD_TOOLSET_GCC     3

#define BUILD_PLATFORM_UNKNOWN 0
#define BUILD_PLATFORM_AMD64   1

#define BUILD_IS_CONFIG_DEBUG ((BUILD_CONFIG == BUILD_CONFIG_DEBUG) || (BUILD_CONFIG == BUILD_CONFIG_RELEASE))
#define BUILD_IS_CONFIG_DIST  ((BUILD_CONFIG == BUILD_CONFIG_RELEASE) || (BUILD_CONFIG == BUILD_CONFIG_DIST))

#define BUILD_IS_SYSTEM_WINDOWS (BUILD_SYSTEM == BUILD_SYSTEM_WINDOWS)
#define BUILD_IS_SYSTEM_MACOSX  (BUILD_SYSTEM == BUILD_SYSTEM_MACOSX)
#define BUILD_IS_SYSTEM_LINUX   (BUILD_SYSTEM == BUILD_SYSTEM_LINUX)

#define BUILD_IS_TOOLSET_MSVC  (BUILD_TOOLSET == BUILD_TOOLSET_MSVC)
#define BUILD_IS_TOOLSET_CLANG (BUILD_TOOLSET == BUILD_TOOLSET_CLANG)
#define BUILD_IS_TOOLSET_GCC   (BUILD_TOOLSET == BUILD_TOOLSET_GCC)

#define BUILD_IS_PLATFORM_AMD64 (BUILD_PLATFORM == BUILD_PLATFORM_AMD64)

namespace CommonBuild
{
	using BuildConfig   = Flags<std::uint16_t>;
	using BuildSystem   = Flags<std::uint16_t>;
	using BuildToolset  = Flags<std::uint16_t>;
	using BuildPlatform = Flags<std::uint16_t>;

	namespace BuildConfigs
	{
		static constexpr BuildConfig Unknown = 0;
		static constexpr BuildConfig Debug   = 1;
		static constexpr BuildConfig Dist    = 2;
	} // namespace BuildConfigs

	namespace BuildSystems
	{
		static constexpr BuildSystem Unknown = 0;
		static constexpr BuildSystem Windows = 1;
		static constexpr BuildSystem Unix    = 2;
		static constexpr BuildSystem MacOSX  = 4;
		static constexpr BuildSystem Linux   = 8;
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
		static constexpr BuildPlatform AMD64   = 1;
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
		return BuildSystems::Windows;
#elif BUILD_SYSTEM == BUILD_SYSTEM_MACOSX
		return BuildSystems::Unix | BuildSystems::MacOSX;
#elif BUILD_SYSTEM == BUILD_SYSTEM_LINUX
		return BuildSystems::Unix | BuildSystems::Linux;
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
#if BUILD_PLATFORM == BUILD_PLATFORM_AMD64
		return BuildPlatforms::AMD64;
#else
		return BuildPlatforms::Unknown;
#endif
	}

	static constexpr BuildConfig   c_Config   = GetBuildConfig();
	static constexpr BuildSystem   c_System   = GetBuildSystem();
	static constexpr BuildToolset  c_Toolset  = GetBuildToolset();
	static constexpr BuildPlatform c_Platform = GetBuildPlatform();

	static constexpr bool c_IsConfigDebug = c_Config.hasFlag(BuildConfigs::Debug);
	static constexpr bool c_IsConfigDist  = c_Config.hasFlag(BuildConfigs::Dist);

	static constexpr bool c_IsSystemWindows = c_System.hasFlag(BuildSystems::Windows);
	static constexpr bool c_IsSystemUnix    = c_System.hasFlag(BuildSystems::Unix);
	static constexpr bool c_IsSystemMacOSX  = c_System.hasFlag(BuildSystems::MacOSX);
	static constexpr bool c_IsSystemLinux   = c_System.hasFlag(BuildSystems::Linux);

	static constexpr bool c_IsToolsetMSVC  = c_Toolste.hasFlag(BuildToolsets::MSVC);
	static constexpr bool c_IsToolsetClang = c_Toolste.hasFlag(BuildToolsets::Clang);
	static constexpr bool c_IsToolsetGCC   = c_Toolste.hasFlag(BuildToolsets::GCC);

	static constexpr bool c_IsPlatformAMD64 = c_Platform.hasFlag(BuildPlatforms::AMD64);
} // namespace CommonBuild