assimppath = "../../external/assimp--3.0.1270"

project "texture_loader"
    location (ProjectDir)
	debugdir (DebugDir) --only in Premake 4.4
    kind "SharedLib"
    language "C++"
    files {"../../resourceloaders/textureloader/include/**.h","../../resourceloaders/textureloader/src/**.cpp"}
    defines {HeartDefines}
    defines {"GWEN_DLL"}
    defines {CommonDefines,SharedLibDefines}
    defines {"TEXTURE_LOADER_COMPILE_DLL"}
    includedirs {HeartIncludeDirs}
    includedirs {"../../resourceloaders/textureloader/include/"}
    includedirs {assimppath.."/include"}
	links {PlatformLibs}
	links { "heartbase", "heartcore", "heartwin32", "crypto", "lua", "libpng", "libtga" }
    links { "assimp" }
	
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        libdirs {TargetDir..DebugCfgName}
        libdirs {assimppath.."lib/assimp_debug-dll_win32"}
		flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        libdirs {TargetDir..ReleaseCfgName}
        libdirs {assimppath.."lib/assimp_release-dll_win32"}
        flags {ReleaseOptions}