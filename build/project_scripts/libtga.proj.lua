
project "libtga"
    location (ProjectDir)
    kind "StaticLib"
    language "C"
    files {"../../libtga/include/**.h","../../libtga/Src/**.c"}
    defines {CommonDefines,SharedLibDefine}
	includedirs {"../../libtga/include"}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {{DebugDefines}}
        flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {{ReleaseDefines}}
        flags {ReleaseOptions}