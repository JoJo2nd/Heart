local vsname = "vc8"
if os.getenv("VS90COMNTOOLS") ~= nil then
    vsname = "vc9"
elseif os.getenv("VS100COMNTOOLS") ~= nil then
    vsname = "vc10"
end

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
    includedirs {"../../external/nvidia-texture-tools/src"}
	links {PlatformLibs}
    links { "nvtt" }
    links "zlib"
	
    configuration (DebugCfgName)
        targetdir (TargetDir..DebugCfgName)
        defines {DebugDefines}
        libdirs {"../../external/nvidia-texture-tools/project/"..vsname.."/Debug (no cuda).Win32/lib"}
        libdirs {TargetDir..DebugCfgName}
        links {HeartLibsDebug}
		flags {DebugOptions}
    configuration (ReleaseCfgName)
        targetdir (TargetDir..ReleaseCfgName)
        defines {ReleaseDefines}
        libdirs {"../../external/nvidia-texture-tools/project/"..vsname.."/Release (no cuda).Win32/lib"}
        libdirs {TargetDir..ReleaseCfgName}
        links {HeartLibsRelease}
        flags {ReleaseOptions}