
project "libtga"
    location (ProjectDir)
    kind "StaticLib"
    language "C"
    files {"../libtga/Include/*.h","../libtga/Src/*.c"}
    defines {CommonDefines}
	includedirs {"../libtga/include"}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {{DebugDefines}}
        flags {"Symbols"}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {{ReleaseDefines}}
        flags {"Optimize"}