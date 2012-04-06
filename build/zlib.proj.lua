
project "zlib"
    location (ProjectDir)
    kind "StaticLib"
    language "C"
    files {"../zlib/Include/*.h","../zlib/Src/*.c"}
    defines {CommonDefines}
	defines {"NO_vsnprintf"}
	includedirs {"../zlib/include"}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {{DebugDefines}}
        flags {"Symbols"}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {{ReleaseDefines}}
        flags {"Optimize"}