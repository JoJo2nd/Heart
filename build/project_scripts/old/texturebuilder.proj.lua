project "texturebuilder"
	location (ProjectDir)
	kind "SharedLib"
	language "C++"
	files {"../engineresourcebuilders/texturebuilder/include/**.h","../engineresourcebuilders/texturebuilder/src/**.cpp"}
	defines {CommonDefines}
	includedirs {
		"../engineresourcebuilders/texturebuilder/include",
		"../libpng/include",
		"../squish"
		}
	includedirs {ResourceBuilderIncludeDirs}
	libdirs {ExternalLibDirs}
	links {ResourceBuilderLibs}
	links {
		"squish",
		"libpng"
		}
	
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