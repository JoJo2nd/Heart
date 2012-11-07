project "heartwin32"
    location (ProjectDir)
    kind "StaticLib"
    language "C++"
    files {"../../heart/include/pal/win32/device/**.h","../../heart/src/pal/win32/heartWin32Unity.cpp"}
    files {"../../heart/src/pal/win32/device/**.cpp"} -- these files are added but not compiled
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