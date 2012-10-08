project "heartwin32"
    location (ProjectDir)
    kind "SharedLib"
    language "C++"
    files {"../../heartwin32/include/**.h","../../heartwin32/src/heartWin32Unity.cpp"}
    files {"../../heartwin32/src/**.cpp"} -- these files are added but not compiled
    defines {HeartDefines}
    defines {"HEART_WIN32_DLL"}
    defines {CommonDefines,SharedLibDefines}
    includedirs {HeartIncludeDirs}
    links {PlatformLibs}
    links {"heartbase","crypto"}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        --flags {"Symbols","Optimize"}
		flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        flags {ReleaseOptions}