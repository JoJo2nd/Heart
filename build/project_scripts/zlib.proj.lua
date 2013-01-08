
project "zlib"
    location (ProjectDir)
    kind "SharedLib"
    language "C"
    files {"../../external/zlib/Include/*.h","../../external/zlib/Src/*.c"}
    excludes {"../../external/zlib/Src/minigzip.c"}
    defines {CommonDefines,SharedLibDefines,"ZLIB_DLL"}
	defines {"NO_vsnprintf"}
	includedirs {"../../external/zlib/include"}
    
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