
SlnName = "memtrack"
BinType = "tools"
DebugSuffix = "_d"
ReleaseSuffix = "_r"
heartBuildRoot=os.getenv("HEART_BUILD_ROOT")
heartRepoRoot=os.getenv("HEART_REPO_ROOT")
heartBinToolRoot=os.getenv("HEART_BIN_TOOL_ROOT")
heartBinGameRoot=os.getenv("HEART_BIN_GAME_ROOT")
heartBinRoot=os.getenv("HEART_BIN_ROOT")
heartProjectRoot="../project_scripts/"
heartProjectCommonRoot="../project_common/"

dofile(heartProjectCommonRoot.."heart_common_proj.lua")

IncludeDirs = {
    "../../tools/memtrack/include",
    {wxWidgetsIncludeDirs},
    "../../external/boost/"
}
LibDirs = {
    {wxWidgetsLibsDirs},
    "../../external/boost/stage/lib"
}
PlatformDefines={
    {wxWidgetsDefines},
    "WIN32",
    "_WIN32",
    "WINDOWS",
    "_WINDOWS",
    "_CRT_SECURE_NO_WARNINGS"
}
DebugDefines={
    "_DEBUG","DEBUG"
}
DebugOptions={
    "Symbols","NoEditAndContinue","NoMinimalRebuild"
}
ReleaseDefines={
    "NDEBUG","RELEASE"
}
ReleaseOptions={
    "Optimize","NoEditAndContinue","NoMinimalRebuild"
}
PlatformLibs={
    "dbghelp"
}
LibsDebug={}
LibsRelease={}

solution (SlnName)
    location (SlnDir)
    configurations ({DebugCfgName, ReleaseCfgName})
    
    configuration (DebugCfgName)
        targetsuffix(DebugSuffix)
        
    configuration (ReleaseCfgName)
        targetsuffix(ReleaseSuffix)
    
    project "memtrack"
        location (ProjectDir)
        debugdir (DebugDir) --only in Premake 4.4
        kind "WindowedApp"
        language "C++"
        files {
            "../../tools/memtrack/include/**.h",
            "../../tools/memtrack/src/**.cpp"}
        --pchheader "../../tools/memtrack/include/precompiled.h"
        pchheader "precompiled.h"
        pchsource "../../tools/memtrack/src/precompiled.cpp"
        defines {PlatformDefines}
        includedirs {IncludeDirs}
        links {PlatformLibs}
        libdirs {LibDirs}
        flags {"WinMain","Unicode"}

        configuration (DebugCfgName)
            targetdir (TargetDir..DebugCfgName)
            defines {DebugDefines}
            flags {DebugOptions}
            postbuildcommands {
                ssub(PostBuildToolDeployCmd, table.splice(HeartCommonVars, {LIBNAME=project().name, TARGETDIR=TargetDir..DebugCfgName, PROJECT=project().name,CONFIG=DebugCfgName}))
            }
            postbuildcommands {
                ssub(DeploywxWidgetsCmd, table.splice(HeartCommonVars, {WXTYPE="ud", PROJECT=project().name,CONFIG=DebugCfgName}))
            }
        configuration (ReleaseCfgName)
            targetdir (TargetDir..ReleaseCfgName)
            defines {ReleaseDefines}
            flags {ReleaseOptions}
            postbuildcommands {
                ssub(PostBuildToolDeployCmd, table.splice(HeartCommonVars, {LIBNAME=project().name, TARGETDIR=TargetDir..ReleaseCfgName, PROJECT=project().name,CONFIG=ReleaseCfgName}))
            }
            postbuildcommands {
                ssub(DeploywxWidgetsCmd, table.splice(HeartCommonVars, {WXTYPE="u", PROJECT=project().name,CONFIG=ReleaseCfgName}))
            }


