project "libogg"
    location (ProjectDir)
    kind "StaticLib"
    language "C"
    files {"../../external/libogg/include/**.h","../../external/libogg/src/**.c"}
    defines {CommonDefines}
	includedirs {"../../external/libogg/include"}
    
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