local PostBuildStr="cd ../../../deploy_scripts\ncall deploy_runtime_libs.bat"

project "heart_testbed"
    location (ProjectDir)
	debugdir (DebugDir) --only in Premake 4.4
    kind "WindowedApp"
    language "C++"
    files {
        "../../testbed/include/**.h",
        "../../testbed/src/**.cpp"}
    pchheader "../../testbed/include/testbed_precompiled.h"
    pchsource "../../testbed/src/testbed_precompiled.cpp"
    defines {HeartDefines}
    defines {"GWEN_DLL"}
    defines {CommonDefines}
    includedirs {HeartIncludeDirs}
    includedirs {"../../testbed/include"}
	links {PlatformLibs}
	links { "heart", "crypto", "lua" }
    flags {"WinMain"}
    postbuildcommands {PostBuildStr}
	
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        --flags {"Symbols","Optimize"}
		flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        flags {ReleaseOptions}