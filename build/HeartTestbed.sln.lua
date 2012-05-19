
SlnName = "HeartTestBed"
DebugCfgName = "Debug"
ReleaseCfgName = "Release"
SlnDir = "built_projects/".._ACTION.."/"..SlnName.."/"
ProjectDir = SlnDir.."projects/"
TargetDir = SlnDir.."bin/"
DebugDir="../bin/game"
HeartIncludeDirs = {
    "../heart/include",
    "../zlib/include",
    "../lua/src",
    "../minizip/include",
    "../crypto/include",
    "../rapidxml-1.13/include",
    "../libvorbis/include",
    "../libogg/include",
    "../gwen/include",
    "../libtga/include"}

HeartDefines= {"HEART_ENGINE","HEART_PLAT_DX11","USE_DL_PREFIX","ONLY_MSPACES"}
PlatformDefines={"WIN32","_WIN32","WINDOWS","_WINDOWS"}
CommonDefines= {{PlatformDefines},"_CRT_SECURE_NO_WARNINGS","HEART_ENGINE_LIB"}
DebugDefines={"_DEBUG","DEBUG"}
ReleaseDefines={"NDEBUG","RELEASE"}

PlatformLibs={"d3d11","d3dcompiler","d3dx11","dxguid","xinput","openAL32"}

PostBuildStr="IF NOT EXIST ..\\..\\..\\..\\..\\bin\\game\\ MKDIR ..\\..\\..\\..\\..\\bin\\game\\ \ncopy /Y $(TargetPath) ..\\..\\..\\..\\..\\bin\\game\\"

solution (SlnName)
    location (SlnDir)
    configurations ({DebugCfgName, ReleaseCfgName})
    
    dofile "testbed.proj.lua" --1st project becomes the start up project
    dofile "zlib.proj.lua"
    dofile "libtga.proj.lua"
    dofile "lua.proj.lua"
    dofile "minizip.proj.lua"
    dofile "crypto.proj.lua"
    dofile "libvorbis.proj.lua"
    dofile "libvorbisfile.proj.lua"
    dofile "libogg.proj.lua"
    dofile "heart.proj.lua"
    -- TODO: fix heart memory heap construction so global constructors are ok
    dofile "gwen.proj.lua"

