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
            postbuildcommands {PostBuildStr..project().name..DebugSuffix}
        configuration (ReleaseCfgName)
            targetdir (TargetDir..ReleaseCfgName)
            defines {{ReleaseDefines}}
            flags {ReleaseOptions}
            postbuildcommands {PostBuildStr..project().name..ReleaseSuffix}