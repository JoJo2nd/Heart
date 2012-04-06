    project "lua"
        location (ProjectDir)
        kind "StaticLib"
        language "C"
        files {"../lua/src/*.h","../lua/src/*.c"}
        excludes {"../lua/src/lua.c","../lua/src/luac.c"}
		includedirs {"../lua/src"}
		defines {CommonDefines}
        
        configuration (DebugCfgName)
            targetdir (TargetDir..DebugCfgName)
			defines {{DebugDefines}}
            flags {"Symbols"}
        configuration (ReleaseCfgName)
			targetdir (TargetDir..ReleaseCfgName)
			defines {{ReleaseDefines}}
            flags {"Optimize"}