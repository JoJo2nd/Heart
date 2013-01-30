
SlnName = "viewer"
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

dofile "../project_common/heart_common_proj.lua"
-- Make as prebuild commnad
-- os.execute("\"..\\deploy_scripts\\deploy_external_libs.bat\"");

-- link against the game version of libs
DebugDir=ssub("$BINGAMEROOT",HeartCommonVars)
HeartLibDir = ssub("$(BUILDROOT)/$ACTION/game/lib/", table.splice(HeartCommonVars,{ACTION=_ACTION,SLNNAME=SlnName}))
--"../built_projects/".._ACTION.."/game/lib/"

debugArgsStr=[[
-p "$(BINGAMEROOT)../tools/plugin/$(CONFIG);$(BINGAMEROOT)./plugin/$(CONFIG);$(BINGAMEROOT)../tools/viewer_plugins/$(CONFIG);$(BINGAMEROOT)viewer_plugins/$(CONFIG)"
]]

myIncludeDirs = {
    {HeartIncludeDirs},
    {wxWidgetsIncludeDirs},
    "../../tools/viewer/include",
    "../../tools/viewer_api/include",
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
    {wxWidgetsDefines},
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
    "viewer_api"
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
        buildoptions { "-Zm200" } -- needs at least Zm127 or boost pops it
        --prebuildcommands "cd ../../../../deploy_scripts\ncall deploy_game_libs_to_tools_bin.bat"

        configuration (DebugCfgName)
            targetdir (ssub("$BINTOOLROOT/$PROJECT/$CONFIG",table.splice(HeartCommonVars,{PROJECT=project().name,CONFIG=DebugCfgName})))
            defines {myDebugDefines}
            debugargs(ssub(debugArgsStr, table.splice(HeartCommonVars, {CONFIG=DebugCfgName})))
            flags {myDebugOptions}
            libdirs(HeartLibDir..DebugCfgName)
            links {myLibsDebug}
            postbuildcommands {
                ssub(PostBuildToolDeployCmd, table.splice(HeartCommonVars, {LIBNAME=project().name, TARGETDIR=TargetDir..DebugCfgName, PROJECT=project().name,CONFIG=DebugCfgName}))
            }
            postbuildcommands {
                ssub(DeploywxWidgetsCmd, table.splice(HeartCommonVars, {WXTYPE="ud", PROJECT=project().name,CONFIG=DebugCfgName}))
            }
        configuration (ReleaseCfgName)
            targetdir (ssub("$BINTOOLROOT/$PROJECT/$CONFIG",table.splice(HeartCommonVars,{PROJECT=project().name,CONFIG=ReleaseCfgName})))
            defines {myReleaseDefines}
            debugargs(ssub(debugArgsStr, table.splice(HeartCommonVars, {CONFIG=ReleaseCfgName})))
            flags {myReleaseOptions}
            libdirs(HeartLibDir..ReleaseCfgName)
            links {myLibsRelease}
            postbuildcommands {
                ssub(PostBuildToolDeployCmd, table.splice(HeartCommonVars, {LIBNAME=project().name, TARGETDIR=TargetDir..ReleaseCfgName, PROJECT=project().name,CONFIG=ReleaseCfgName}))
            }
            postbuildcommands {
                ssub(DeploywxWidgetsCmd, table.splice(HeartCommonVars, {WXTYPE="u", PROJECT=project().name,CONFIG=ReleaseCfgName}))
            }
    
    dofile "viewer_api.proj.lua"
    dofile "viewer_texture_plugin.proj.lua"
    dofile "viewer_mesh_plugin.proj.lua"