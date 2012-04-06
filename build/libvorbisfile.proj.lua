project "libvorbisfile"
    location (ProjectDir)
    kind "StaticLib"
    language "C"
    files {"../libvorbis/include/vorbisfile.h","../libvorbis/lib/vorbisfile.c"}
    defines {CommonDefines}
	includedirs {"../libvorbis/include","../libogg/include"}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {{DebugDefines}}
        flags {"Symbols"}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {{ReleaseDefines}}
        flags {"Optimize"}