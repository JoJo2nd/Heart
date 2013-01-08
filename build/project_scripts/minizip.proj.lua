project "minizip"
	location (ProjectDir)
	kind "StaticLib"
	language "C"
	files {"../../external/minizip/include/*.h","../../external/minizip/src/*.c"}
	excludes {"../../external/minizip/src/minizip.c", "../../external/minizip/src/miniunz.c"}
	defines {CommonDefines,SharedLibDefines,"ZLIB_DLL"}
    defines {"NO_vsnprintf"}
	includedirs {"../../external/minizip/include","../../external/zlib/include"}
    links {"zlib"}
	
configuration (DebugCfgName)
	targetdir (TargetDir..DebugCfgName)
	defines {{DebugDefines}}
	flags {DebugOptions}
    StrVars=table.splice(HeartCommonVars, {
            TARGETDIR=TargetDir..DebugCfgName,
            LIBNAME=project().name,
            PROJECT=project().name,
            CONFIG=DebugCfgName,
        })
    postbuildcommands(ssub(PostBuildDeployCmd,StrVars))
configuration (ReleaseCfgName)
	targetdir (TargetDir..ReleaseCfgName)
	defines {{ReleaseDefines}}
	flags {ReleaseOptions}
    StrVars=table.splice(HeartCommonVars, {
        TARGETDIR=TargetDir..ReleaseCfgName,
        LIBNAME=project().name,
        PROJECT=project().name,
        CONFIG=ReleaseCfgName,
    })
    postbuildcommands(ssub(PostBuildDeployCmd,StrVars))