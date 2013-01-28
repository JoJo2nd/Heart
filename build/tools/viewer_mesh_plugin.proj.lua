local vsname = "vc8"
if os.getenv("VS90COMNTOOLS") ~= nil then
    vsname = "vc9"
elseif os.getenv("VS100COMNTOOLS") ~= nil then
    vsname = "vc10"
end

-- link against the game version of libs
meshpluginHeartLibDir = ssub("$(BUILDROOT)/$ACTION/game/lib/", table.splice(HeartCommonVars,{ACTION=_ACTION,SLNNAME=SlnName}))
meshpluginHeartLibs= {
}
meshpluginPrebuildCmd=[[
echo Copying AssImp libs
robocopy "$(REPOROOT)external/assimp--3.0.1270-sdk/bin/assimp_debug-dll_Win32" "$(BINTOOLROOT)plugin/debug" *.dll *.pdb /XO /XX /njh /njs /ndl /nc /ns /np
robocopy "$(REPOROOT)external/assimp--3.0.1270-sdk/bin/assimp_release-dll_Win32" "$(BINTOOLROOT)plugin/release" *.dll /XO /XX /njh /njs /ndl /nc /ns /np
exit /B 0
]]

project "mesh_plugin"
    location (ProjectDir)
    kind "SharedLib"
    language "C++"
    files {
        "../../tools/mesh_plugin/include/**.h",
        "../../tools/mesh_plugin/src/**.cpp"
    }
    --pchheader "precompiled.h"
    --pchsource "../../tools/viewer_api/src/precompiled.cpp"
    defines {
        {CommonDefines},
        {HeartDefines},
        {"_DLL","SHARED_LIB","PLUGIN_COMPILE_DLL"},
        {PlatformDefines},
        {wxWidgetsDefines},
    }
    includedirs {
        "../../external/crypto/include",
        "../../external/rapidxml-1.13/include",
        {wxWidgetsIncludeDirs},
        "../../tools/viewer_api/include",
        "../../tools/mesh_plugin/include",
        "../../external/boost/",
        "../../external/assimp--3.0.1270-sdk/include",
        {HeartIncludeDirs},
    }
    links {
        {PlatformLibs},
        "dbghelp",
        "shlwapi",
        "viewer_api",
        "assimp",
    }
    libdirs {
        {LibDirs},
        {wxWidgetsLibsDirs},
        "../../external/boost/stage/lib",
    }
    flags {"Unicode"}
    buildoptions { "-Zm116" } -- needs at least Zm116 or boost pops it
    prebuildcommands {
        ssub(meshpluginPrebuildCmd, HeartCommonVars),
    }

    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {
            {DebugDefines},
        }
        flags {
            {DebugOptions},
        }
        libdirs {
            meshpluginHeartLibDir..DebugCfgName,
            ssub("$(REPOROOT)external/assimp--3.0.1270-sdk/lib/assimp_debug-dll_Win32", HeartCommonVars),
        }
        links {
            appendSuffixToTableEntries(meshpluginHeartLibs, DebugSuffix),
            appendSuffixToTableEntries(HeartLibsDebug, DebugSuffix),
        }
        postbuildcommands {
            ssub(PostBuildToolPluginDeployCmd, table.splice(HeartCommonVars,{LIBNAME=project().name, TARGETDIR=TargetDir..DebugCfgName, PROJECT=project().name,CONFIG=DebugCfgName})) 
        }
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines { 
            {ReleaseDefines},
        }
        flags {
            {ReleaseOptions},
        }
        libdirs {
            meshpluginHeartLibDir..ReleaseCfgName,
            ssub("$(REPOROOT)external/assimp--3.0.1270-sdk/lib/assimp_release-dll_Win32", HeartCommonVars),
        }
        links {
            appendSuffixToTableEntries(meshpluginHeartLibs, ReleaseSuffix),
            appendSuffixToTableEntries(HeartLibsDebug, ReleaseSuffix),
        }
        postbuildcommands {
            ssub(PostBuildToolPluginDeployCmd, table.splice(HeartCommonVars,{LIBNAME=project().name, TARGETDIR=TargetDir..ReleaseCfgName, PROJECT=project().name,CONFIG=ReleaseCfgName})) 
        }