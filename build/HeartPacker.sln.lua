SlnName = "HeartPacker"
DebugCfgName = "Debug"
ReleaseCfgName = "Release"
SlnDir = "built_projects/".._ACTION.."/"..SlnName.."/"
ProjectDir = SlnDir.."projects/"
TargetDir = SlnDir.."bin/"
BinDir = "../bin/game/"
HeartIncludeDirs = {"../heart/include","../zlib/include","../lua/src","../minizip/include","../crypto/include","../rapidxml-1.13/include","../libvorbis/include","../libogg/include"}
ResourceBuilderIncludeDirs = {"../gamedatabaselib/include","../external/boost",{HeartIncludeDirs}}
wxWidgetsIncludes={"../external/wxWidgets-2.9.2/include","../external/wxWidgets-2.9.2/include/msvc"}

HeartDefines= {"HEART_ENGINE","HEART_PLAT_DX11","USE_DL_PREFIX","ONLY_MSPACES"}
PlatformDefines={"WIN32","_WIN32","WINDOWS","_WINDOWS"}
CommonDefines= {{PlatformDefines},{HeartDefines},"_CRT_SECURE_NO_WARNINGS","HEART_ENGINE_LIB","HEART_PACKER"}
DebugDefines={"_DEBUG","DEBUG"}
ReleaseDefines={"NDEBUG","RELEASE"}

PlatformLibs={"d3d11","d3dcompiler","d3dx11","dxguid","xinput","openAL32"}

ExternalLibDirs={"../external/boost/stage/lib","../external/wxWidgets-2.9.2/lib/vc_lib"}
HeartLibs={"zlib","crypto","libogg","libvorbis","libvorbisfile","lua","heart","minizip",{PlatformLibs}}
ResourceBuilderLibs={"gamedatabaselib",{HeartLibs}}

ToolPostBuildStr="IF NOT EXIST ..\\..\\..\\..\\..\\bin\\tools\\ MKDIR ..\\..\\..\\..\\..\\bin\\tools\\ \ncopy /Y $(TargetPath) ..\\..\\..\\..\\..\\bin\\tools\\"
DebugPluginPostBuildStr="IF NOT EXIST ..\\..\\..\\..\\..\\bin\\tools\\debug_plugins\\ MKDIR ..\\..\\..\\..\\..\\bin\\tools\\debug_plugins\\ \ncopy /Y $(TargetPath) ..\\..\\..\\..\\..\\bin\\tools\\debug_plugins\\"
ReleasePluginPostBuildStr="IF NOT EXIST ..\\..\\..\\..\\..\\bin\\tools\\release_plugins\\ MKDIR ..\\..\\..\\..\\..\\bin\\tools\\release_plugins\\ \ncopy /Y $(TargetPath) ..\\..\\..\\..\\..\\bin\\tools\\release_plugins\\"

solution (SlnName)
    location (SlnDir)
    configurations ({DebugCfgName, ReleaseCfgName})
    
	dofile "heartpacker.proj.lua"
    dofile "zlib.proj.lua"
    dofile "lua.proj.lua"
    dofile "minizip.proj.lua"
    dofile "crypto.proj.lua"
    dofile "libvorbis.proj.lua"
    dofile "libvorbisfile.proj.lua"
    dofile "libogg.proj.lua"
    dofile "heart.proj.lua"
	dofile "libpng.proj.lua"
	dofile "squish.proj.lua"
	dofile "freetype.proj.lua"
	--dofile "nvtristrip.proj.lua"
	dofile "gamedatabaselib.proj.lua"
	--resource builders
	dofile "texturebuilder.proj.lua"
	dofile "fontbuilder.proj.lua"
	dofile "materialbuilder.proj.lua"
	dofile "shaderbuilder.proj.lua"
	dofile "oggstreambuilder.proj.lua"
	dofile "oggsoundbank.proj.lua"
	dofile "worldobjectbuilder.proj.lua"