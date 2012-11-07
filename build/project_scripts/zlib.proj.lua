
project "zlib"
    location (ProjectDir)
    kind "SharedLib"
    language "C"
    files {"../../external/zlib/Include/*.h","../../external/zlib/Src/*.c"}
    excludes {"../../external/zlib/Src/minigzip.c"}
    defines {CommonDefines,SharedLibDefines,"ZLIB_DLL"}
	defines {"NO_vsnprintf"}
	includedirs {"../../external/zlib/include"}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {{DebugDefines}}
        flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {{ReleaseDefines}}
        flags {ReleaseOptions}