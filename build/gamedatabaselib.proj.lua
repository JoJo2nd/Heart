project "gamedatabaselib"
	location (ProjectDir)
	kind "SharedLib"
	language "C++"
	files {"../gamedatabaselib/include/*.h","../gamedatabaselib/src/GameDatabaseLibUnity.cpp"}
	defines {CommonDefines}
	defines {
		"_UNICODE",
		"UNICODE"
	}
	includedirs {"../gamedatabaselib/include"}
	includedirs {ResourceBuilderIncludeDirs}
	libdirs {ExternalLibDirs}
	links{"crypto"}
	flags{"Unicode"}
	
	postbuildcommands (ToolPostBuildStr)
	
configuration (DebugCfgName)
	targetsuffix "_d"
	targetdir (TargetDir..DebugCfgName)
	defines {{DebugDefines}}
	flags {"Symbols"}
configuration (ReleaseCfgName)
	targetsuffix "_r"
	targetdir (TargetDir..ReleaseCfgName)
	defines {{ReleaseDefines}}
	flags {"Optimize"}