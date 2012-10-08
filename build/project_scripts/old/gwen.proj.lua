
project "gwen"
	location (ProjectDir)
	defines { "GWEN_COMPILE_STATIC" }
	defines {CommonDefines}
	includedirs {"../../gwen/include","../../heartbase/include"}
	files { "../../gwen/src/**.*", "../../gwen/include/gwen/**.*" }
	flags { "Unicode", "NoMinimalRebuild", "NoEditAndContinue", "NoPCH", "No64BitChecks", "NoRTTI" }
	kind "StaticLib"
	language "C++"
	
	configuration "Release"
		targetdir (TargetDir..ReleaseCfgName)
		defines {{ReleaseDefines}}
		flags {"Optimize", "FloatFast"}
		
	configuration "Debug"
		targetdir (TargetDir..DebugCfgName)
		defines {{DebugDefines}}
		flags {"Symbols"}
		