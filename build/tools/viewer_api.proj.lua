
-- link against the game version of libs
viewerHeartLibDir = ssub("$(BUILDROOT)/$ACTION/game/lib/", table.splice(HeartCommonVars,{ACTION=_ACTION,SLNNAME=SlnName}))
viewerHeartLibs= {
    "crypto"
}

project "viewer_api"
    location (ProjectDir)
    kind "SharedLib"
    language "C++"
    files {
        "../../tools/viewer_api/include/**.h",
        "../../tools/viewer_api/include/impl/**.h",
        "../../tools/viewer_api/src/**.cpp"
    }
    --pchheader "precompiled.h"
    --pchsource "../../tools/viewer_api/src/precompiled.cpp"
    defines {
        {CommonDefines},
        {"_DLL","SHARED_LIB","VAPI_COMPILE_DLL"},
        {PlatformDefines},
    }
    includedirs {
        "../../external/crypto/include",
        "../../external/rapidxml-1.13/include",
        {wxWidgetsIncludeDirs},
        "../../tools/viewer_api/include",
        "../../external/boost/",
    }
    links {
        "dbghelp",
        "shlwapi",
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
        libdirs(viewerHeartLibDir..DebugCfgName)
        links {
            appendSuffixToTableEntries(viewerHeartLibs, DebugSuffix)
        }
        postbuildcommands {
            ssub(PostBuildToolDeployCmd, table.splice(HeartCommonVars,{LIBNAME=project().name, TARGETDIR=TargetDir..DebugCfgName, PROJECT="viewer",CONFIG=DebugCfgName})) 
        }
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines { 
            {ReleaseDefines},
        }
        flags {
            {ReleaseOptions},
        }
        libdirs(viewerHeartLibDir..ReleaseCfgName)
        links {
            appendSuffixToTableEntries(viewerHeartLibs, ReleaseSuffix)
        }
        postbuildcommands {
            ssub(PostBuildToolDeployCmd, table.splice(HeartCommonVars,{LIBNAME=project().name, TARGETDIR=TargetDir..ReleaseCfgName, PROJECT="viewer",CONFIG=ReleaseCfgName})) 
        }