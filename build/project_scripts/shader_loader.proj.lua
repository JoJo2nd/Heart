project "shader_loader"
    location (ProjectDir)
	debugdir (DebugDir) --only in Premake 4.4
    kind "SharedLib"
    language "C++"
    files {"../../resourceloaders/shaderloader/include/**.h","../../resourceloaders/shaderloader/src/**.cpp"}
    defines {HeartDefines}
    defines {"GWEN_DLL"}
    defines {CommonDefines,SharedLibDefines}
    defines {"SHADER_LOADER_COMPILE_DLL"}
    includedirs {HeartIncludeDirs}
    includedirs {"../../resourceloaders/shaderloader/include/"}
	links {PlatformLibs}
	
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        libdirs {TargetDir..DebugCfgName}
        links {HeartLibsDebug}
		flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        libdirs {TargetDir..ReleaseCfgName}
        links {HeartLibsRelease}
        flags {ReleaseOptions}