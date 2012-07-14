project "modelbuilder"
	location (ProjectDir)
	kind "SharedLib"
	language "C++"
	files {"../engineresourcebuilders/modelbuilder/include/**.h","../engineresourcebuilders/modelbuilder/src/**.cpp"}
	defines {CommonDefines}
	includedirs {
		"../engineresourcebuilders/modelbuilder/include",
        "../external/assimp/include",
		}
	includedirs {ResourceBuilderIncludeDirs}
	libdirs {ExternalLibDirs}
	links {ResourceBuilderLibs}
    links {"assimp"}
	
configuration (DebugCfgName)
	postbuildcommands (DebugPluginPostBuildStr)
	targetsuffix "_d"
	targetdir (TargetDir..DebugCfgName)
	defines {{DebugDefines}}
	flags {"Symbols"}
    libdirs {"../external/assimp/lib/assimp_debug-noboost-st_Win32"}
configuration (ReleaseCfgName)
	postbuildcommands (ReleasePluginPostBuildStr)
	targetsuffix "_r"
	targetdir (TargetDir..ReleaseCfgName)
	defines {{ReleaseDefines}}
	flags {"Optimize"}
    libdirs {"../external/assimp/lib/assimp_release-noboost-st_Win32"}
    