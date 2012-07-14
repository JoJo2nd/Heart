
SlnName = "NewTestBed"
DebugCfgName = "Debug"
ReleaseCfgName = "Release"
SlnOutput = "built_projects/".._ACTION.."/"..SlnName.."/"
SlnDir = "../" .. SlnOutput
ProjectDir = SlnDir.."projects/"
TargetDir = SlnDir.."bin/"
DebugDir="../../bin/game"
HeartIncludeDirs = {
    "../../heartbase/include",
    "../../heartwin32/include",
    "../../heartcore/include",
    "../../heartcore/gwen/include",
    "../../zlib/include",
    "../../lua/src",
    "../../minizip/include",
    "../../crypto/include",
    "../../rapidxml-1.13/include",
    "../../libvorbis/include",
    "../../libogg/include",
    "../../libtga/include"}

HeartDefines= {"HEART_ENGINE","HEART_PLAT_DX11","USE_DL_PREFIX","ONLY_MSPACES"}
PlatformDefines={"WIN32","_WIN32","WINDOWS","_WINDOWS"}
CommonDefines= {{PlatformDefines},"_CRT_SECURE_NO_WARNINGS","HEART_ENGINE_LIB"}
SharedLibDefines = {"_DLL","SHARED_LIB","HEART_DLL"}
DebugDefines={"_DEBUG","DEBUG"}
ReleaseDefines={"NDEBUG","RELEASE"}

PlatformLibs={"d3d11","d3dcompiler","d3dx11","dxguid","xinput","openAL32"}

PostBuildStr="IF NOT EXIST ..\\..\\..\\..\\..\\bin\\game\\ MKDIR ..\\..\\..\\..\\..\\bin\\game\\ \ncopy /Y $(TargetPath) ..\\..\\..\\..\\..\\bin\\game\\"

solution (SlnName)
    location (SlnOutput)
    configurations ({DebugCfgName, ReleaseCfgName})
    
    dofile "project_scripts/bootloader.proj.lua" --1st project becomes the start up project
    dofile "project_scripts/zlib.proj.lua"
    dofile "project_scripts/libtga.proj.lua"
    dofile "project_scripts/lua.proj.lua"
    dofile "project_scripts/minizip.proj.lua"
    dofile "project_scripts/crypto.proj.lua"
    dofile "project_scripts/libvorbis.proj.lua"
    dofile "project_scripts/libvorbisfile.proj.lua"
    dofile "project_scripts/libogg.proj.lua"
    dofile "project_scripts/heartbase.proj.lua"
    dofile "project_scripts/heartwin32.proj.lua"
    dofile "project_scripts/heartcore.proj.lua"
    -- TODO: fix heart memory heap construction so global constructors are ok
    -- dofile "project_scripts/gwen.proj.lua"
    dofile "project_scripts/testbed.proj.lua"

