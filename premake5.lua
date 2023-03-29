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
		common:outDirs()

		includedirs({ "CommonBuild/" })
		files({ "CommonBuild/**" })
		removefiles({ "*.DS_Store" })

		common:addActions()