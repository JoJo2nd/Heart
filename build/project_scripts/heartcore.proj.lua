project "heartcore"
    location (ProjectDir)
    kind "SharedLib"
    language "C++"
    files {"../../heartcore/include/**.h","../../heartcore/src/heartCoreUnity.cpp"}
    files { "../../heartcore/gwen/src/**.*", "../../heartcore/gwen/include/**.*" }
    files {"../../heartcore/src/**.cpp"} -- these files are added but not compiled
    defines {HeartDefines}
    defines {"HEART_CORE_DLL","GWEN_COMPILE_DLL"}
    defines {CommonDefines,SharedLibDefines}
    includedirs {HeartIncludeDirs}
    includedirs { "../../heartcore/gwen/src/**.*", "../../heartcore/gwen/include/**.*" }
    links {"heartbase","heartwin32","crypto","lua","zlib","minizip","libogg","libvorbis","libvorbisfile"}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        --flags {"Symbols","Optimize"}
		flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        flags {ReleaseOptions}