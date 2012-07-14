project "gameld48_23"
    location (ProjectDir)
	debugdir (DebugDir) --only in Premake 4.4
    kind "WindowedApp"
    language "C++"
    files {"../game/include/**.h","../game/src/**.cpp"}
    defines {HeartDefines}
    defines {CommonDefines}
    includedirs {HeartIncludeDirs}
    includedirs {"../game/include"}
	links {PlatformLibs}
	links {"zlib","crypto","libogg","libvorbis","libvorbisfile","lua","heart","minizip"}
	flags {"WinMain"}
	pchheader "precompiled.h"
	pchsource "precompiled.cpp"
	
	postbuildcommands (PostBuildStr)
    
    configuration (DebugCfgName)
		targetsuffix "_d"
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        flags {"Symbols"}
    configuration (ReleaseCfgName)
		targetsuffix "_r"
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        flags {"Optimize"}