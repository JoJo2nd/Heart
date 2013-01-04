
-- link against the game version of libs
texpluginHeartLibDir = "../built_projects/".._ACTION.."/game/lib/"
texpluginHeartLibs= {
}

project "texture_plugin"
    location (ProjectDir)
    kind "SharedLib"
    language "C++"
    files {
        "../../tools/texture_plugin/include/**.h",
        "../../tools/texture_plugin/src/**.cpp"
    }
    --pchheader "precompiled.h"
    --pchsource "../../tools/viewer_api/src/precompiled.cpp"
    defines {
        {CommonDefines},
        {"_DLL","SHARED_LIB","PLUGIN_COMPILE_DLL"},
        {PlatformDefines},
        {wxWidgetsDefines},
    }
    includedirs {
        "../../external/crypto/include",
        "../../external/rapidxml-1.13/include",
        {wxWidgetsIncludeDirs},
        "../../tools/viewer_api/include",
        "../../tools/texture_plugin/include",
        "../../external/boost/",
    }
    links {
        "dbghelp",
        "shlwapi",
        "viewer_api"
    }
    libdirs {
        {wxWidgetsLibsDirs},
        "../../external/boost/stage/lib",
    }
    flags {"Unicode"}
    buildoptions { "-Zm116" } -- needs at least Zm116 or boost pops it

    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {
            {DebugDefines},
        }
        flags {
            {DebugOptions},
        }
        libdirs(texpluginHeartLibDir..DebugCfgName)
        links {
            appendSuffixToTableEntries(texpluginHeartLibs, DebugSuffix)
        }
        postbuildcommands {PostBuildStr..project().name..DebugSuffix.." plugin"}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines { 
            {ReleaseDefines},
        }
        flags {
            {ReleaseOptions},
        }
        libdirs(texpluginHeartLibDir..ReleaseCfgName)
        links {
            appendSuffixToTableEntries(texpluginHeartLibs, ReleaseSuffix)
        }
        postbuildcommands {PostBuildStr..project().name..ReleaseSuffix.." plugin"}