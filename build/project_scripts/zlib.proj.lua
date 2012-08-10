
project "zlib"
    location (ProjectDir)
    kind "SharedLib"
    language "C"
    files {"../../zlib/Include/*.h","../../zlib/Src/*.c"}
    excludes {"../../zlib/Src/minigzip.c"}
    defines {CommonDefines,SharedLibDefines,"ZLIB_DLL"}
	defines {"NO_vsnprintf"}
	includedirs {"../../zlib/include"}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {{DebugDefines}}
        flags {"Symbols"}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {{ReleaseDefines}}
        flags {"Optimize"}