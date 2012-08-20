project "heart_packer"
	location (ProjectDir)
	kind "WindowedApp"
	language "C++"
	files {"../heartpacker/include/**.h","../heartpacker/src/**.cpp"}
	excludes {"../heartpacker/src/UIPropertyID.cpp"}
    defines {HeartDefines}
    defines {CommonDefines}
	defines {
		"_UNICODE",
		"UNICODE"
	}
    includedirs {HeartIncludeDirs}
    includedirs {
		"../heartpacker/include",
		"../external/boost",
		{wxWidgetsIncludes},
		"../gamedatabaselib/include"}
	libdirs {ExternalLibDirs}
	links {PlatformLibs}
	links {"zlib","crypto","libogg","libvorbis","libvorbisfile","lua","heart","minizip","gwen_static"}
	links {"gamedatabaselib"}
	flags {
		"WinMain",
		"Unicode"}
	
	postbuildcommands (ToolPostBuildStr)
	
configuration (DebugCfgName)
	targetsuffix "_d"
	targetdir (TargetDir..DebugCfgName)
	defines {{DebugDefines}}
	flags {DebugOptions}
configuration (ReleaseCfgName)
	targetsuffix "_r"
	targetdir (TargetDir..ReleaseCfgName)
	defines {{ReleaseDefines}}
	flags {ReleaseOptions}