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
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {{ReleaseDefines}}
        flags {ReleaseOptions}