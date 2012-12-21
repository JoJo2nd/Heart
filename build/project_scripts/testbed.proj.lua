--local PostBuildCmd="cd ../../../deploy_scripts\ncall deploy_external_libs.bat\ncd ../../../deploy_scripts\ncall deploy_lib.bat "
local PostBuildCmd="cd ../../../deploy_scripts\ncall deploy_lib.bat "

project "heart_testbed"
    location (ProjectDir)
    debugdir (DebugDir) --only in Premake 4.4
    kind "WindowedApp"
    language "C++"
    files {
        "../../testbed/include/**.h",
        "../../testbed/src/**.cpp"}
    --pchheader "../../testbed/include/testbed_precompiled.h"
    pchsource "../../testbed/src/testbed_precompiled.cpp"
    pchheader "testbed_precompiled.h"
    --pchsource "testbed_precompiled.cpp"
    defines {HeartDefines}
    defines {"GWEN_DLL"}
    defines {CommonDefines}
    includedirs {HeartIncludeDirs}
    includedirs {"../../testbed/include"}
    links {PlatformLibs}
    links { "heart", "crypto", "lua" }
    flags {"WinMain"}
    postbuildcommands {PostBuildCmd..project().name}

    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        --flags {"Symbols","Optimize"}
        flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        flags {ReleaseOptions}