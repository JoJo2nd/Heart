project "heart"
    location (ProjectDir)
    kind "SharedLib"
    language "C++"
    files {
        "../../heart/include/**.h",
        "../../heart/src/heartCoreUnity.cpp",
        "../../heart/src/heartBaseUnity.cpp",
        "../../heart/src/pal/win32/heartWin32Unity.cpp"}
    files {"../../heart/src/**.cpp"} -- these files are added but not compiled
    defines {HeartDefines}
    defines {"HEART_COMPILE_DLL","GWEN_COMPILE_DLL"}
    defines {CommonDefines,SharedLibDefines}
    includedirs {HeartIncludeDirs}
    includedirs { "../../heartcore/gwen/src/**.*", "../../heartcore/gwen/include/**.*" }
    links {"crypto","lua","zlib","minizip","libogg","libvorbis","libvorbisfile"}
    links {PlatformLibs}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        --flags {"Symbols","Optimize"}
		flags {DebugOptions}
        postbuildcommands {PostBuildStr..project().name..DebugSuffix}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        flags {ReleaseOptions}
        postbuildcommands {PostBuildStr..project().name..ReleaseSuffix}