
DebugCfgName = "Debug"
ReleaseCfgName = "Release"
SlnOutput = "../built_projects/".._ACTION.."/"..BinType.."/"..SlnName.."/"
SlnDir = SlnOutput
ProjectDir = SlnDir.."../projects/"
TargetDir = SlnDir.."../lib/"
DebugDir="../../bin/"..BinType
HeartPlatformIncludeDirs = {
    os.getenv("DXSDK_DIR").."/Include"}
HeartIncludeDirs = {
    HeartPlatformIncludeDirs,
    --common
    "../../heart/include",
    "../../heart/include/pal/win32",
    "../../heart/gwen/include",
    "../../external/zlib/include",
    "../../external/lua/src",
    "../../external/libpng/include",
    "../../external/minizip/include",
    "../../external/crypto/include",
    "../../external/rapidxml-1.13/include",
    "../../external/libvorbis/include",
    "../../external/libogg/include",
    "../../external/libtga/include",
    "../../external/openal/include"}

HeartDefines= {"HEART_ENGINE","HEART_PLAT_DX11","USE_DL_PREFIX","ONLY_MSPACES","HEART_LUA_LIBS"}
PlatformDefines={"WIN32","_WIN32","WINDOWS","_WINDOWS"}
CommonDefines= {{PlatformDefines},"_CRT_SECURE_NO_WARNINGS","HEART_ENGINE_LIB"}
SharedLibDefines = {"_DLL","SHARED_LIB","HEART_DLL"}
DebugDefines={"_DEBUG","DEBUG"}
--DebugOptions={"Symbols","NoEditAndContinue","NoMinimalRebuild"}
DebugOptions={"Symbols","NoMinimalRebuild"}
ReleaseDefines={"NDEBUG","RELEASE"}
ReleaseOptions={"Optimize","NoEditAndContinue","NoMinimalRebuild","Symbols"}
--ReleaseOptions={"Symbols"}

LibDirs={
    "../../external/openal/libs/Win32",
    os.getenv("DXSDK_DIR").."/Lib/x86"}
PlatformLibs={"d3d11","d3dcompiler","d3dx11","dxguid","xinput","openAL32","dbghelp"}
HeartLibsDebug={
    "heart", 
    "crypto", 
    "lua",
    "zlib",
    "libpng", 
    "libtga"}
HeartLibsRelease={
    "heart", 
    "crypto", 
    "lua", 
    "zlib",
    "libpng", 
    "libtga"}

if BinType == "game" then
    PostBuildStr="cd ../../../../deploy_scripts\ncall deploy_lib.bat "
else
    PostBuildStr="cd ../../../../deploy_scripts\ncall deploy_tool.bat "
end