project "fontbuilder"
	location (ProjectDir)
	kind "SharedLib"
	language "C++"
	files {"../engineresourcebuilders/fontbuilder/include/**.h","../engineresourcebuilders/fontbuilder/src/**.cpp"}
	defines {CommonDefines}
	includedirs {
		"../engineresourcebuilders/fontbuilder/include",
		"../freetype-2.4.8/include",
		"../libpng/include",
		"../zlib/include"
		}
	includedirs {ResourceBuilderIncludeDirs}
	libdirs {ExternalLibDirs}
	links {ResourceBuilderLibs}
	links {"freetype"}
	
configuration (DebugCfgName)
	postbuildcommands (DebugPluginPostBuildStr)
	targetsuffix "_d"
	targetdir (TargetDir..DebugCfgName)
	defines {{DebugDefines}}
	flags {"Symbols"}
configuration (ReleaseCfgName)
	postbuildcommands (ReleasePluginPostBuildStr)
	targetsuffix "_r"
	targetdir (TargetDir..ReleaseCfgName)
	defines {{ReleaseDefines}}
	flags {"Optimize"}