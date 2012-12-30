local PostBuildCmd="cd ../../../../deploy_scripts\ncall deploy_tool.bat "

SlnName = "memtrack"
BinType = "tools"
DebugSuffix = "_d"
ReleaseSuffix = "_r"

DebugCfgName = "Debug"
ReleaseCfgName = "Release"
SlnOutput = "built_projects/".._ACTION.."/tools/"..SlnName.."/"
SlnDir = "../" .. SlnOutput
ProjectDir = SlnDir.."../projects/"
TargetDir = SlnDir.."../lib/"
DebugDir="../../bin/"..BinType
IncludeDirs = {
    "../../tools/memtrack/include",
    "../../external/wxWidgets-2.9.2/include",
    "../../external/wxWidgets-2.9.2/include/msvc",
    "../../external/boost/"}
LibDirs = {
    "../../external/wxWidgets-2.9.2/lib/vc_lib",
    "../../external/boost/stage/lib"}

PlatformDefines={"WIN32","_WIN32","WINDOWS","_WINDOWS","_CRT_SECURE_NO_WARNINGS"}
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
    
    postbuildcommands {PostBuildStr}
    
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
            postbuildcommands {PostBuildCmd..project().name..DebugSuffix.." "..project().name}
        configuration (ReleaseCfgName)
            targetdir (TargetDir..ReleaseCfgName)
            defines {ReleaseDefines}
            flags {ReleaseOptions}
            postbuildcommands {PostBuildCmd..project().name..ReleaseSuffix.." "..project().name}


