
SlnName = "memtrack"
BinType = "tools"
DebugSuffix = "_d"
ReleaseSuffix = "_r"

dofile "../project_common/heart_common_proj.lua"

DebugCfgName = "Debug"
ReleaseCfgName = "Release"
SlnOutput = "built_projects/".._ACTION.."/tools/"..SlnName.."/"
SlnDir = "../" .. SlnOutput
ProjectDir = SlnDir.."../projects/"
TargetDir = SlnDir.."../lib/"
DebugDir="../../bin/"..BinType
IncludeDirs = {
    "../../tools/memtrack/include",
    {wxWidgetsIncludeDirs},
    "../../external/boost/"}
LibDirs = {
    {wxWidgetsLibsDirs},
    "../../external/boost/stage/lib"}

PlatformDefines={
    {wxWidgetsDefines},
    "WIN32",
    "_WIN32",
    "WINDOWS",
    "_WINDOWS",
    "_CRT_SECURE_NO_WARNINGS"
}
DebugDefines={"_DEBUG","DEBUG"}
DebugOptions={"Symbols","NoEditAndContinue","NoMinimalRebuild"}
ReleaseDefines={"NDEBUG","RELEASE"}
ReleaseOptions={"Optimize","NoEditAndContinue","NoMinimalRebuild"}
--ReleaseOptions={"Symbols"}

PlatformLibs={"dbghelp"}
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
            --flags {"Symbols","Optimize"}
            flags {DebugOptions}
            postbuildcommands {PostBuildStr..project().name..DebugSuffix.." "..project().name}
            postbuildcommands {
                "call deploy_wxwidgets_libs.bat ud "..string.gsub(ToolDeployDir, "%$(%w+)", {project=project().name, config=DebugCfgName}),
            }
        configuration (ReleaseCfgName)
            targetdir (TargetDir..ReleaseCfgName)
            defines {ReleaseDefines}
            flags {ReleaseOptions}
            postbuildcommands {PostBuildStr..project().name..ReleaseSuffix.." "..project().name}
            postbuildcommands {
                "call deploy_wxwidgets_libs.bat u "..string.gsub(ToolDeployDir, "%$(%w+)", {project=project().name, config=ReleaseCfgName}),
            }


