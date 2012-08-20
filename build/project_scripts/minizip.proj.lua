project "minizip"
	location (ProjectDir)
	kind "StaticLib"
	language "C"
	files {"../../minizip/include/*.h","../../minizip/src/*.c"}
	excludes {"../../minizip/src/minizip.c", "../../minizip/src/miniunz.c"}
	defines {CommonDefines,SharedLibDefines,"ZLIB_DLL"}
    defines {"NO_vsnprintf"}
	includedirs {"../../minizip/include","../../zlib/include"}
    links {"zlib"}
	
configuration (DebugCfgName)
	targetdir (TargetDir..DebugCfgName)
	defines {{DebugDefines}}
	flags {DebugOptions}
configuration (ReleaseCfgName)
	targetdir (TargetDir..ReleaseCfgName)
	defines {{ReleaseDefines}}
	flags {ReleaseOptions}