project "squish"
	location (ProjectDir)
	kind "StaticLib"
	language "C++"
	files {"../squish/*.h","../squish/*.cpp"}
	defines {CommonDefines}
	includedirs {"../squish"}
	
configuration (DebugCfgName)
	targetdir (TargetDir..DebugCfgName)
	defines {{DebugDefines}}
	flags {"Symbols"}
configuration (ReleaseCfgName)
	targetdir (TargetDir..ReleaseCfgName)
	defines {{ReleaseDefines}}
	flags {"Optimize"}