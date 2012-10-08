project "heart_testbed"
    location (ProjectDir)
	debugdir (DebugDir) --only in Premake 4.4
    kind "SharedLib"
    language "C++"
    files {"../../testbed/include/**.h","../../testbed/src/**.cpp"}
    defines {HeartDefines}
    defines {"GWEN_DLL"}
    defines {CommonDefines,SharedLibDefines}
    includedirs {HeartIncludeDirs}
    includedirs {"../../testbed/include"}
	links {PlatformLibs}
	links { "heartbase", "heartcore", "heartwin32", "crypto", "lua" }
    --links { "heartbase", "heartcore", "crypto", "lua" }
	
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        --flags {"Symbols","Optimize"}
		flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        flags {ReleaseOptions}