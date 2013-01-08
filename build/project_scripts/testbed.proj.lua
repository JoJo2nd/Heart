
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

    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        flags {DebugOptions}
        StrVars=table.splice(HeartCommonVars, {
            TARGETDIR=TargetDir..DebugCfgName,
            LIBNAME=project().name,
            PROJECT=project().name,
            CONFIG=DebugCfgName,
        })
        postbuildcommands(ssub(PostBuildDeployCmd,StrVars))
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        flags {ReleaseOptions}
        StrVars=table.splice(HeartCommonVars, {
            TARGETDIR=TargetDir..ReleaseCfgName,
            LIBNAME=project().name,
            PROJECT=project().name,
            CONFIG=ReleaseCfgName,
        })
        postbuildcommands(ssub(PostBuildDeployCmd,StrVars))