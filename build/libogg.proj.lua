project "libogg"
    location (ProjectDir)
    kind "StaticLib"
    language "C"
    files {"../libogg/include/**.h","../libogg/src/**.c"}
    defines {CommonDefines}
	includedirs {"../libogg/include"}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {{DebugDefines}}
        flags {"Symbols"}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {{ReleaseDefines}}
        flags {"Optimize"}