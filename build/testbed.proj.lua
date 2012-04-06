project "heart_testbed"
    location (ProjectDir)
	debugdir (DebugDir) --only in Premake 4.4
    kind "WindowedApp"
    language "C++"
    files {"../testbed/include/**.h","../testbed/src/**.cpp"}
    defines {HeartDefines}
    defines {CommonDefines}
    includedirs {HeartIncludeDirs}
    includedirs {"../testbed/include"}
	links {PlatformLibs}
	links {"zlib","crypto","libogg","libvorbis","libvorbisfile","lua","heart","minizip"}
	flags {"WinMain"}
	
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