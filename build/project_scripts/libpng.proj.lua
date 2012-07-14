project "libpng"
	location (ProjectDir)
	kind "StaticLib"
	language "C"
	files {"../libpng/include/*.h","../libpng/src/*.c"}
	excludes {"../minizip/src/minizip.c", "../minizip/src/miniunz.c"}
	defines {CommonDefines}
	includedirs {"../libpng/include","../zlib/include"}
	
configuration (DebugCfgName)
	targetdir (TargetDir..DebugCfgName)
	defines {{DebugDefines}}
	flags {"Symbols"}
configuration (ReleaseCfgName)
	targetdir (TargetDir..ReleaseCfgName)
	defines {{ReleaseDefines}}
	flags {"Optimize"}