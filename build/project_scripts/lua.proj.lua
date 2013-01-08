project "lua"
    location (ProjectDir)
    kind "SharedLib"
    language "C"
    files {"../../external/lua/src/*.h","../../external/lua/src/*.c"}
    excludes {"../../external/lua/src/lua.c","../../external/lua/src/luac.c"}
    includedirs {"../../external/lua/src"}
    defines {
        CommonDefines,
        "LUA_BUILD_AS_DLL",
        "HEART_LUA_LIBS"
        }
    
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