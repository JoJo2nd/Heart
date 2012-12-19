
project "libtga"
    location (ProjectDir)
    kind "StaticLib"
    language "C"
    files {"../../external/libtga/include/**.h","../../external/libtga/Src/**.c"}
    defines {CommonDefines,SharedLibDefine}
	includedirs {"../../external/libtga/include"}
    postbuildcommands {PostBuildStr..project().name}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {{DebugDefines}}
        flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {{ReleaseDefines}}
        flags {ReleaseOptions}