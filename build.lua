local pkg       = premake.extensions.pkg
local scriptDir = pkg:scriptDir()
local premake   = pkg.builders.premake

local buildTool = premake:setup("CommonBuild", "amd64", { "Debug", "Release", "Dist" }, scriptDir, "build", "%{targetname}/%{config}", "--build-pkg")
buildTool:mapConfigs({
	Debug = {
		config  = "Debug",
		targets = { CommonBuild = { path = "CommonBuild/", outputFiles = pkg:libName("CommonBuild", true) } }
	},
	Release = {
		config  = "Release",
		targets = { CommonBuild = { path = "CommonBuild/", outputFiles = pkg:libName("CommonBuild", true) } }
	},
	Dist = {
		config  = "Dist",
		targets = { CommonBuild = { path = "CommonBuild/", outputFiles = pkg:libName("CommonBuild", true) } }
	}
})
buildTool:build()
buildTool:cleanTemp()