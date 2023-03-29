workspace("CommonBuild")
	location("build/")
	common:setConfigsAndPlatforms()
	common:addCoreDefines()

	cppdialect("C++20")
	rtti("Off")
	exceptionhandling("Off")
	flags("MultiProcessorCompile")

	project("CommonBuild")
		location("%{wks.location}/")
		warnings("Extra")

		kind("ConsoleApp")
		targetdir("%{wks.location}/CommonBuild/%{cfg.buildcfg}")
		objdir("%{wks.location}/CommonBuild/%{cfg.buildcfg}")

		includedirs({ "CommonBuild/" })
		files({ "CommonBuild/**" })
		removefiles({ "*.DS_Store" })

		common:addActions()