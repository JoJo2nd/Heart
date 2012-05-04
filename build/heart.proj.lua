project "heart"
    location (ProjectDir)
    kind "StaticLib"
    language "C++"
    files {"../heart/include/**.h","../heart/src/HeartUnity.cpp"}
    defines {HeartDefines}
    defines {CommonDefines}
    includedirs {HeartIncludeDirs}
    
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        --flags {"Symbols","Optimize"}
		flags {"Symbols"}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        flags {"Optimize"}