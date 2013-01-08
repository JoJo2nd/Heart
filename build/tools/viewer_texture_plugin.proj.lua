local vsname = "vc8"
if os.getenv("VS90COMNTOOLS") ~= nil then
    vsname = "vc9"
elseif os.getenv("VS100COMNTOOLS") ~= nil then
    vsname = "vc10"
end

-- link against the game version of libs
texpluginHeartLibDir = ssub("$(BUILDROOT)/$ACTION/game/lib/", table.splice(HeartCommonVars,{ACTION=_ACTION,SLNNAME=SlnName}))
texpluginHeartLibs= {
}
texpluginPrebuildCmd=[[
echo Copying nvtt libs"
cd "$(REPOROOT)build/deploy_scripts"
call deploy_nvtt.bat "$(BINTOOLROOT)/plugin/debug"
call deploy_nvtt.bat "$(BINTOOLROOT)/plugin/release"
echo Copying FreeImage libs
robocopy "$(REPOROOT)external/freeimage/dist" "$(BINTOOLROOT)plugin/debug" freeimage.dll /XO /XX /njh /njs /ndl /nc /ns /np
robocopy "$(REPOROOT)external/freeimage/dist" "$(BINTOOLROOT)plugin/release" freeimage.dll /XO /XX /njh /njs /ndl /nc /ns /np
exit /B 0
]]

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
        "../../external/nvidia-texture-tools/src",
        "../../external/freeimage/dist",
    }
    links {
        "dbghelp",
        "shlwapi",
        "viewer_api",
        "nvtt",
        "freeimage",
    }
    libdirs {
        {wxWidgetsLibsDirs},
        "../../external/boost/stage/lib",
        "../../external/nvidia-texture-tools/project/"..vsname.."/Release (no cuda).Win32/lib",
        "../../external/freeimage/dist",
    }
    flags {"Unicode"}
    buildoptions { "-Zm116" } -- needs at least Zm116 or boost pops it
    prebuildcommands {
        ssub(texpluginPrebuildCmd, HeartCommonVars),
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
            texpluginHeartLibDir..DebugCfgName,
        }
        links {
            appendSuffixToTableEntries(texpluginHeartLibs, DebugSuffix)
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
            texpluginHeartLibDir..ReleaseCfgName,
        }
        links {
            appendSuffixToTableEntries(texpluginHeartLibs, ReleaseSuffix)
        }
        postbuildcommands {
            ssub(PostBuildToolPluginDeployCmd, table.splice(HeartCommonVars,{LIBNAME=project().name, TARGETDIR=TargetDir..ReleaseCfgName, PROJECT=project().name,CONFIG=ReleaseCfgName})) 
        }