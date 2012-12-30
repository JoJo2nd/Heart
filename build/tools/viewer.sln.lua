
SlnName = "viewer"
BinType = "tools"
DebugSuffix = "_d"
ReleaseSuffix = "_r"

dofile "../project_common/heart_common_proj.lua"
os.execute("\"..\\deploy_scripts\\deploy_external_libs.bat\"");

DebugCfgName = "Debug"
ReleaseCfgName = "Release"
SlnOutput = "built_projects/".._ACTION.."/tools/"..SlnName.."/"
SlnDir = "../" .. SlnOutput
ProjectDir = SlnDir.."../projects/"
TargetDir = SlnDir.."../lib/"
DebugDir="../../bin/"..BinType
-- link against the game version of libs
HeartLibDir = "../built_projects/".._ACTION.."/game/lib/"

myIncludeDirs = {
    {HeartIncludeDirs},
    {wxWidgetsIncludeDirs},
    "../../tools/viewer/include",
    "../../external/boost/",
    }
myLibDirs = {
    {LibDirs},
    {wxWidgetsLibsDirs},
    "../../external/boost/stage/lib",
    }
myDefines={
    {CommonDefines},
    {HeartDefines},
    "_SCL_SECURE_NO_WARNINGS"
}
myDebugDefines={
    {myDefines},
    {DebugDefines},
}
myDebugOptions={
    {DebugOptions},
}
myReleaseDefines={
    {myDefines},
    {ReleaseDefines},
}
myReleaseOptions={
    {ReleaseOptions},
}

myPlatformLibs={
    {PlatformLibs},
}
myLibsDebug={
    {appendSuffixToTableEntries(HeartLibsDebug, DebugSuffix)},
}
myLibsRelease={
    {appendSuffixToTableEntries(HeartLibsRelease, ReleaseSuffix)},
}

solution (SlnName)
    location (SlnDir)
    configurations ({DebugCfgName, ReleaseCfgName})
    
    configuration (DebugCfgName)
        targetsuffix(DebugSuffix)
        
    configuration (ReleaseCfgName)
        targetsuffix(ReleaseSuffix)
    
    project "viewer"
        location (ProjectDir)
        debugdir (DebugDir) --only in Premake 4.4
        kind "WindowedApp"
        language "C++"
        files {
            "../../tools/viewer/include/**.h",
            "../../tools/viewer/src/**.cpp"
        }
        pchheader "precompiled.h"
        pchsource "../../tools/viewer/src/precompiled.cpp"
        defines {myPlatformDefines}
        includedirs {myIncludeDirs}
        links {myPlatformLibs}
        libdirs {myLibDirs}
        flags {"WinMain","Unicode"}
        buildoptions { "-Zm116" } -- needs at least Zm116 or boost pops it
        --prebuildcommands "cd ../../../../deploy_scripts\ncall deploy_game_libs_to_tools_bin.bat"

        configuration (DebugCfgName)
            targetdir (TargetDir..DebugCfgName)
            defines {myDebugDefines}
            flags {myDebugOptions}
            libdirs(HeartLibDir..DebugCfgName)
            links {myLibsDebug}
            postbuildcommands {PostBuildStr..project().name..DebugSuffix.." "..project().name}
        configuration (ReleaseCfgName)
            targetdir (TargetDir..ReleaseCfgName)
            defines {myReleaseDefines}
            flags {myReleaseOptions}
            libdirs(HeartLibDir..ReleaseCfgName)
            links {myLibsRelease}
            postbuildcommands {PostBuildStr..project().name..ReleaseSuffix.." "..project().name}


