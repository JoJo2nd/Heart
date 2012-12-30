
project "libtga"
    location (ProjectDir)
    kind "StaticLib"
    language "C"
    files {"../../external/libtga/include/**.h","../../external/libtga/Src/**.c"}
    defines {CommonDefines,SharedLibDefine}
	includedirs {"../../external/libtga/include"}
    
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