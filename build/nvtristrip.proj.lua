project "nvtristrip"
	location (ProjectDir)
	kind "StaticLib"
	language "C++"
	files {"../nvtristrip/include/**.h","../nvtristrip/src/**.cpp"}
	defines {CommonDefines}
	includedirs {"../nvtristrip/include"}
	
configuration (DebugCfgName)
	targetdir (TargetDir..DebugCfgName)
	defines {{DebugDefines}}
	flags {"Symbols"}
configuration (ReleaseCfgName)
	targetdir (TargetDir..ReleaseCfgName)
	defines {{ReleaseDefines}}
	flags {"Optimize"}