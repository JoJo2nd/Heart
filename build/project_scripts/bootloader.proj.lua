project "bootloader"
    location (ProjectDir)
	debugdir (DebugDir) --only in Premake 4.4
    kind "WindowedApp"
    language "C++"
    files {"../../bootloader/include/**.h","../../bootloader/src/**.cpp"}
    defines {HeartDefines}
    defines {CommonDefines}
    includedirs {HeartIncludeDirs}
    includedirs {"../../bootloader/include"}
	links {PlatformLibs}
	flags {"WinMain"}
    
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