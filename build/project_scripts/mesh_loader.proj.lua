assimppath = "../../external/assimp--3.0.1270-sdk/"

project "mesh_loader"
    location (ProjectDir)
	debugdir (DebugDir) --only in Premake 4.4
    kind "SharedLib"
    language "C++"
    files {"../../resourceloaders/meshloader/include/**.h","../../resourceloaders/meshloader/src/**.cpp"}
    defines {HeartDefines}
    defines {"GWEN_DLL"}
    defines {CommonDefines,SharedLibDefines}
    defines {
        "MESH_LOADER_COMPILE_DLL",
        "_HAS_ITERATOR_DEBUGGING=0"}
    includedirs {HeartIncludeDirs}
    includedirs {"../../resourceloaders/meshloader/include/"}
    includedirs {assimppath.."/include"}
	links {PlatformLibs}
    links { "assimp" }
	
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        libdirs {TargetDir..DebugCfgName}
        libdirs {assimppath.."lib/assimp_debug-dll_win32"}
        links {HeartLibsDebug}
		flags {DebugOptions}
        postbuildcommands {PostBuildStrPlugin..project().name..DebugSuffix}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        libdirs {TargetDir..ReleaseCfgName}
        libdirs {assimppath.."lib/assimp_release-dll_win32"}
        links {HeartLibsRelease}
        flags {ReleaseOptions}
        postbuildcommands {PostBuildStrPlugin..project().name..ReleaseSuffix}