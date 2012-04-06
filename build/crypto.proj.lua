    project "crypto"
        location (ProjectDir)
        kind "StaticLib"
        language "C"
        files {"../crypto/include/*.h","../crypto/src/*.c"}
        defines {CommonDefines}
		includedirs {"../crypto/include"}
        
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {{DebugDefines}}
        flags {"Symbols"}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {{ReleaseDefines}}
        flags {"Optimize"}