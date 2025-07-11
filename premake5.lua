newoption({
	trigger     = "build-pkg",
	description = "Builds only the package"
})

workspace("CommonBuild")
if _OPTIONS["build-pkg"] then
	location("build/")
end
	common:addConfigs()
	common:addBuildDefines()

	cppdialect("C++20")
	rtti("Off")
	exceptionhandling("On")
	flags("MultiProcessorCompile")

if not _OPTIONS["build-pkg"] then
	startproject("Tests")
end

	project("CommonBuild")
		location("%{wks.location}/")
		warnings("Extra")

		kind("StaticLib")
if not _OPTIONS["build-pkg"] then
		common:outDirs()
else
		targetdir("%{wks.location}/CommonBuild/%{cfg.buildcfg}")
		objdir("%{wks.location}/CommonBuild/%{cfg.buildcfg}")
end

		includedirs({ "Inc/" })
		files({
			"Inc/**",
			"Src/**"
		})
		removefiles({ "*.DS_Store" })

		pkgdeps({ "mimalloc" })

		common:addActions()

if not _OPTIONS["build-pkg"] then
	project("Tests")
		location("Tests/")
		warnings("Extra")

		kind("ConsoleApp")
		common:outDirs()

		includedirs({ "Tests/" })
		files({ "Tests/**" })
		removefiles({ "*.DS_Store" })

		links({ "CommonBuild" })
		externalincludedirs({ "Inc/" })

		common:addActions()
end
