
project "libvorbis"
    location (ProjectDir)
    kind "StaticLib"
    language "C"
    files {"../../external/libvorbis/include/**.h","../../external/libvorbis/lib/**.c"}
    excludes {
        "../../external/libvorbis/lib/vorbisfile.c",
        "../../external/libvorbis/lib/vorbisenc.c",
        "../../external/libvorbis/include/vorbisfile.h",
        "../../external/libvorbis/include/vorbisenc.h",
        "../../external/libvorbis/lib/psytune.c",
        "../../external/libvorbis/lib/tone.c"}
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