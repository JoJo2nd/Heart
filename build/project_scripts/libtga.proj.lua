
project "libtga"
    location (ProjectDir)
    kind "SharedLib"
    language "C"
    files {"../../libtga/include/*.h","../../libtga/Src/*.c"}
    defines {CommonDefines,SharedLibDefine}
	includedirs {"../../libtga/include"}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {{DebugDefines}}
        flags {"Symbols"}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {{ReleaseDefines}}
        flags {"Optimize"}