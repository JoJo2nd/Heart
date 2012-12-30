
-- SlnName = "viewer"
-- BinType = "tools"
-- DebugSuffix = "_d"
-- ReleaseSuffix = "_r"

-- DebugCfgName = "Debug"
-- ReleaseCfgName = "Release"
-- SlnOutput = "built_projects/".._ACTION.."/tools/"..SlnName.."/"
-- SlnDir = "../" .. SlnOutput
-- ProjectDir = SlnDir.."../projects/"
-- TargetDir = SlnDir.."../lib/"
-- DebugDir="../../bin/"..BinType
-- -- link against the game version of libs
-- HeartLibDir = "../built_projects/".._ACTION.."/game/lib/"

-- myIncludeDirs = {
    -- {HeartIncludeDirs},
    -- {wxWidgetsIncludeDirs},
    -- "../../tools/viewer/include",
    -- "../../external/boost/",
    -- }
-- myLibDirs = {
    -- {LibDirs},
    -- {wxWidgetsLibsDirs},
    -- "../../external/boost/stage/lib",
    -- }
-- myDefines={
    -- {CommonDefines},
    -- {HeartDefines},
    -- {wxWidgetsDefines},
    -- "_SCL_SECURE_NO_WARNINGS"
-- }
-- myDebugDefines={
    -- {myDefines},
    -- {DebugDefines},
-- }
-- myDebugOptions={
    -- {DebugOptions},
-- }
-- myReleaseDefines={
    -- {myDefines},
    -- {ReleaseDefines},
-- }
-- myReleaseOptions={
    -- {ReleaseOptions},
-- }

-- myPlatformLibs={
    -- {PlatformLibs},
-- }
-- myLibsDebug={
    -- {appendSuffixToTableEntries(HeartLibsDebug, DebugSuffix)},
-- }
-- myLibsRelease={
    -- {appendSuffixToTableEntries(HeartLibsRelease, ReleaseSuffix)},
-- }

project "viewer_api"
    location (ProjectDir)
    kind "SharedLib"
    language "C++"
    files {
        "../../tools/viewer_api/include/**.h",
        "../../tools/viewer_api/include/impl/**.h",
        "../../tools/viewer_api/src/**.cpp"
    }
    pchheader "precompiled.h"
    pchsource "../../tools/viewer_api/src/precompiled.cpp"
    defines {myPlatformDefines}
    includedirs {myIncludeDirs}
    links {myPlatformLibs}
    libdirs {myLibDirs}
    flags {"Unicode"}
    buildoptions { "-Zm116" } -- needs at least Zm116 or boost pops it

    configuration (DebugCfgName)
        targetdir (string.gsub(ToolDeployDir, "%$(%w+)", {project=project().name, config=DebugCfgName}))
        defines {myDebugDefines}
        flags {myDebugOptions}
        libdirs(HeartLibDir..DebugCfgName)
        links {myLibsDebug}
    configuration (ReleaseCfgName)
        targetdir (string.gsub(ToolDeployDir, "%$(%w+)", {project=project().name, config=ReleaseCfgName}))
        defines {myReleaseDefines}
        flags {myReleaseOptions}
        libdirs(HeartLibDir..ReleaseCfgName)
        links {myLibsRelease}