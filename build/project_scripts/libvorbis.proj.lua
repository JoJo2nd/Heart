
project "libvorbis"
    location (ProjectDir)
    kind "StaticLib"
    language "C"
    files {"../../libvorbis/include/**.h","../../libvorbis/lib/**.c"}
    excludes {"../../libvorbis/lib/vorbisfile.c","../../libvorbis/lib/vorbisenc.c","../../libvorbis/include/vorbisfile.h","../../libvorbis/include/vorbisenc.h","../../libvorbis/lib/psytune.c","../../libvorbis/lib/tone.c"}
    defines {CommonDefines}
	includedirs {"../../libvorbis/include","../../libogg/include"}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {{DebugDefines}}
        flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {{ReleaseDefines}}
        flags {ReleaseOptions}