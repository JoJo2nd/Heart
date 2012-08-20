project "heartbase"
    location (ProjectDir)
    kind "SharedLib"
    language "C++"
    files {"../../heartbase/include/**.h","../../heartbase/src/heartBaseUnity.cpp"}
    files {"../../heartbase/src/**.cpp"} -- these files are added but not compiled
    defines {HeartDefines}
    defines {"HEART_BASE_DLL"}
    defines {CommonDefines,SharedLibDefines}
    includedirs {HeartIncludeDirs}
    links {"crypto"}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        --flags {"Symbols","Optimize"}
		flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        flags {ReleaseOptions}