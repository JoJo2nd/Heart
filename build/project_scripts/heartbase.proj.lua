project "heartbase"
    location (ProjectDir)
    kind "StaticLib"
    language "C++"
    files {"../../heart/include/base/**.h","../../heart/src/heartBaseUnity.cpp"}
    files {"../../heart/src/base/**.cpp"} -- these files are added but not compiled
    defines {HeartDefines}
    defines {"HEART_COMPILE_DLL"}
    defines {CommonDefines}
    includedirs {HeartIncludeDirs}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        --flags {"Symbols","Optimize"}
		flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        flags {ReleaseOptions}