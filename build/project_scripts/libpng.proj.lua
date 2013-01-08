project "libpng"
    location (ProjectDir)
    kind "StaticLib"
    language "C"
    files {"../../external/libpng/include/*.h","../../external/libpng/src/*.c"}
    defines {CommonDefines}
    includedirs {"../../external/libpng/include","../../external/zlib/include"}

    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {{DebugDefines}}
        libdirs {TargetDir..DebugCfgName}
        links {"zlib_d"}
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
        libdirs {TargetDir..ReleaseCfgName}
        links {"zlib_r"}
        flags {ReleaseOptions}
        StrVars=table.splice(HeartCommonVars, {
            TARGETDIR=TargetDir..ReleaseCfgName,
            LIBNAME=project().name,
            PROJECT=project().name,
            CONFIG=ReleaseCfgName,
        })
        postbuildcommands(ssub(PostBuildDeployCmd,StrVars))