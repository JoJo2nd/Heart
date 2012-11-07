    project "crypto"
        location (ProjectDir)
        kind "SharedLib"
        language "C"
        files {"../../external/crypto/include/*.h","../../external/crypto/src/*.c"}
        defines {CommonDefines,SharedLibDefines}
        defines {"CRYPTO_COMPILE_DLL"}
		includedirs {"../../external/crypto/include"}
        
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {{DebugDefines}}
        flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {{ReleaseDefines}}
        flags {ReleaseOptions}