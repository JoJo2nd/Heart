project "oggsoundbank"
	location (ProjectDir)
	kind "SharedLib"
	language "C++"
	files {"../engineresourcebuilders/oggsoundbank/include/**.h","../engineresourcebuilders/oggsoundbank/src/**.cpp"}
	defines {CommonDefines}
	includedirs {"../engineresourcebuilders/oggsoundbank/include"}
	includedirs {ResourceBuilderIncludeDirs}
	libdirs {ExternalLibDirs}
	links {ResourceBuilderLibs}
	
configuration (DebugCfgName)
	postbuildcommands (DebugPluginPostBuildStr)
	targetsuffix "_d"
	targetdir (TargetDir..DebugCfgName)
	defines {{DebugDefines}}
	flags {"Symbols"}
configuration (ReleaseCfgName)
	postbuildcommands (ReleasePluginPostBuildStr)
	targetsuffix "_r"
	targetdir (TargetDir..ReleaseCfgName)
	defines {{ReleaseDefines}}
	flags {"Optimize"}