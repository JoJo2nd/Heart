    project "lua"
        location (ProjectDir)
        kind "SharedLib"
        language "C"
        files {"../../lua/src/*.h","../../lua/src/*.c"}
        excludes {"../../lua/src/lua.c","../../lua/src/luac.c"}
		includedirs {"../../lua/src"}
		defines {CommonDefines,"LUA_BUILD_AS_DLL"}
        
        configuration (DebugCfgName)
            targetdir (TargetDir..DebugCfgName)
			defines {{DebugDefines}}
            flags {"Symbols"}
        configuration (ReleaseCfgName)
			targetdir (TargetDir..ReleaseCfgName)
			defines {{ReleaseDefines}}
            flags {"Optimize"}