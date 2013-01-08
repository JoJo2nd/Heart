assimppath = "../../external/assimp--3.0.1270-sdk/"

meshloaderPreBuildCmd=[[
echo Copying assimp libs
robocopy $(REPOROOT)external/assimp--3.0.1270-sdk/bin/assimp_release-dll_win32 "$(BINGAMEROOT)/plugin" "*.dll" "*.pdb" /XO /XX /njh /njs /ndl /nc /ns /np /nfl /v
robocopy $(REPOROOT)external/assimp--3.0.1270-sdk/bin/assimp_debug-dll_win32 "$(BINGAMEROOT)/plugin" "*.dll" "*.pdb" /XO /XX /njh /njs /ndl /nc /ns /np /nfl /v
]]

project "mesh_loader"
    location (ProjectDir)
	debugdir (DebugDir) --only in Premake 4.4
    kind "SharedLib"
    language "C++"
    files {"../../resourceloaders/meshloader/include/**.h","../../resourceloaders/meshloader/src/**.cpp"}
    defines {HeartDefines}
    defines {"GWEN_DLL"}
    defines {CommonDefines,SharedLibDefines}
    defines {
        "MESH_LOADER_COMPILE_DLL",
        "_HAS_ITERATOR_DEBUGGING=0"}
    includedirs {HeartIncludeDirs}
    includedirs {"../../resourceloaders/meshloader/include/"}
    includedirs {assimppath.."/include"}
	links {PlatformLibs}
    links { "assimp" }
    postbuildcommands(ssub(meshloaderPreBuildCmd, HeartCommonVars))
	
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        libdirs {TargetDir..DebugCfgName}
        libdirs {assimppath.."lib/assimp_debug-dll_win32"}
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
        libdirs {TargetDir..ReleaseCfgName}
        libdirs {assimppath.."lib/assimp_release-dll_win32"}
        links {HeartLibsRelease}
        flags {ReleaseOptions}
        StrVars=table.splice(HeartCommonVars, {
            TARGETDIR=TargetDir..ReleaseCfgName,
            LIBNAME=project().name,
            PROJECT=project().name,
            CONFIG=ReleaseCfgName,
        })
        postbuildcommands(ssub(PostBuildPluginDeployCmd,StrVars))