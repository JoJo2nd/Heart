project "minizip"
	location (ProjectDir)
	kind "StaticLib"
	language "C"
	files {"../minizip/include/*.h","../minizip/src/*.c"}
	excludes {"../minizip/src/minizip.c", "../minizip/src/miniunz.c"}
	defines {CommonDefines}
	includedirs {"../minizip/include","../zlib/include"}
	
configuration (DebugCfgName)
	targetdir (TargetDir..DebugCfgName)
	defines {{DebugDefines}}
	flags {"Symbols"}
configuration (ReleaseCfgName)
	targetdir (TargetDir..ReleaseCfgName)
	defines {{ReleaseDefines}}
	flags {"Optimize"}