newoption({
	trigger     = "build-tests",
	description = "Builds tests"
})

workspace("CommonBuild")
	location("build/")
	common:addConfigs()
	common:addBuildDefines()

	cppdialect("C++20")
	rtti("Off")
	exceptionhandling("On")
	flags("MultiProcessorCompile")

	startproject("Tests")

	project("CommonBuild")
		location("%{wks.location}/")
		warnings("Extra")

		kind("StaticLib")
		targetdir("%{wks.location}/CommonBuild/%{cfg.buildcfg}")
		objdir("%{wks.location}/CommonBuild/%{cfg.buildcfg}")

		includedirs({ "Inc/" })
		files({
			"Inc/**",
			"Src/**"
		})
		removefiles({ "*.DS_Store" })

		pkgdeps({ "mimalloc" })

		common:addActions()

if _OPTIONS["build-tests"] then
	project("Tests")
		location("%{wks.location}/")
		warnings("Extra")

		kind("ConsoleApp")
		targetdir("%{wks.location}/CommonBuild/%{cfg.buildcfg}")
		objdir("%{wks.location}/CommonBuild/%{cfg.buildcfg}")

		includedirs({ "Tests/" })
		files({ "Tests/**" })
		removefiles({ "*.DS_Store" })

		links({ "CommonBuild" })
		externalincludedirs({ "Inc/" })

		common:addActions()
end
