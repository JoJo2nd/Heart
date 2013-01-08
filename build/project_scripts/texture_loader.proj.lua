local vsname = "vc8"
if os.getenv("VS90COMNTOOLS") ~= nil then
    vsname = "vc9"
elseif os.getenv("VS100COMNTOOLS") ~= nil then
    vsname = "vc10"
end

texloaderPreBuildCmd=[[
echo Copying nvtt libs
cd "$(REPOROOT)build/deploy_scripts"
call deploy_nvtt.bat "$(BINGAMEROOT)/plugin"
]]

project "texture_loader"
    location (ProjectDir)
	debugdir (DebugDir) --only in Premake 4.4
    kind "SharedLib"
    language "C++"
    files {"../../resourceloaders/textureloader/include/**.h","../../resourceloaders/textureloader/src/**.cpp"}
    defines {HeartDefines}
    defines {"GWEN_DLL"}
    defines {CommonDefines,SharedLibDefines}
    defines {"TEXTURE_LOADER_COMPILE_DLL"}
    includedirs {HeartIncludeDirs}
    includedirs {"../../resourceloaders/textureloader/include/"}
    includedirs {"../../external/nvidia-texture-tools/src"}
	links {PlatformLibs}
    links { "nvtt" }
    links "zlib"
    postbuildcommands(ssub(texloaderPreBuildCmd, HeartCommonVars))
	
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        libdirs {"../../external/nvidia-texture-tools/project/"..vsname.."/Debug (no cuda).Win32/lib"}
        libdirs {TargetDir..DebugCfgName}
        links {HeartLibsDebug}
		flags {DebugOptions}
        StrVars=table.splice(HeartCommonVars, {
            TARGETDIR=TargetDir..DebugCfgName,
            LIBNAME=project().name,
            PROJECT=project().name,
            CONFIG=DebugCfgName,
        })
        postbuildcommands(ssub(PostBuildPluginDeployCmd,StrVars))
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        libdirs {"../../external/nvidia-texture-tools/project/"..vsname.."/Release (no cuda).Win32/lib"}
        libdirs {TargetDir..ReleaseCfgName}
        links {HeartLibsRelease}
        flags {ReleaseOptions}
        StrVars=table.splice(HeartCommonVars, {
            TARGETDIR=TargetDir..ReleaseCfgName,
            LIBNAME=project().name,
            PROJECT=project().name,
            CONFIG=ReleaseCfgName,
        })
        postbuildcommands(ssub(PostBuildPluginDeployCmd,StrVars))