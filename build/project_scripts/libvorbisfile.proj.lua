project "libvorbisfile"
    location (ProjectDir)
    kind "StaticLib"
    language "C"
    files {"../../external/libvorbis/include/vorbisfile.h","../../external/libvorbis/lib/vorbisfile.c"}
    defines {CommonDefines}
	includedirs {"../../external/libvorbis/include","../../external/libogg/include"}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {{DebugDefines}}
        flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {{ReleaseDefines}}
        flags {ReleaseOptions}