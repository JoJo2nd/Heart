local vsname = "vc8"
if os.getenv("VS90COMNTOOLS") ~= nil then
    vsname = "vc9"
elseif os.getenv("VS100COMNTOOLS") ~= nil then
    vsname = "vc10"
end

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
        "echo Copying nvtt libs",
        "IF NOT exist \"../../../../bin/tools/plugin/debug\" mkdir \"../../bin/tools/plugin/debug\"",
        "IF NOT exist \"../../../../bin/tools/plugin/release\" mkdir \"../../bin/tools/plugin/release\"",
        "cd ../../../../deploy_scripts",
        "call deploy_nvtt.bat \"../../bin/tools/plugin/debug\"",
        "call deploy_nvtt.bat \"../../bin/tools/plugin/release\"",
        "robocopy \"../../external/freeimage/dist\" \"../../bin/tools/plugin/debug\" freeimage.dll /XO /XX /njh /njs /ndl /nc /ns /np",
        "robocopy \"../../external/freeimage/dist\" \"../../bin/tools/plugin/release\" freeimage.dll /XO /XX /njh /njs /ndl /nc /ns /np",
        "exit /B 0",
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
        postbuildcommands {PostBuildStr..project().name..DebugSuffix.." plugin"}
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
        postbuildcommands {PostBuildStr..project().name..ReleaseSuffix.." plugin"}