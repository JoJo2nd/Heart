
DebugCfgName = "Debug"
ReleaseCfgName = "Release"
SlnOutput = "built_projects/".._ACTION.."/"..SlnName.."/"
SlnDir = "../" .. SlnOutput
ProjectDir = SlnDir.."../projects/"
TargetDir = SlnDir.."../lib/"
DebugDir="../../bin/"..BinType
HeartIncludeDirs = {
    "../../heartbase/include",
    "../../heartwin32/include",
    "../../heartcore/include",
    "../../heartcore/gwen/include",
    "../../zlib/include",
    "../../lua/src",
    "../../libpng/include",
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
DebugOptions={"Symbols","NoEditAndContinue","NoMinimalRebuild"}
ReleaseDefines={"NDEBUG","RELEASE"}
ReleaseOptions={"Optimize","NoEditAndContinue","NoMinimalRebuild"}
--ReleaseOptions={"Symbols"}

PlatformLibs={"d3d11","d3dcompiler","d3dx11","dxguid","xinput","openAL32","dbghelp"}
HeartLibsDebug={
    "heartbase"..DebugSuffix, 
    "heartcore"..DebugSuffix, 
    "heartwin32"..DebugSuffix, 
    "crypto"..DebugSuffix, 
    "lua"..DebugSuffix, 
    "libpng"..DebugSuffix, 
    "libtga"..DebugSuffix}
HeartLibsRelease={
    "heartbase"..ReleaseSuffix, 
    "heartcore"..ReleaseSuffix, 
    "heartwin32"..ReleaseSuffix, 
    "crypto"..ReleaseSuffix, 
    "lua"..ReleaseSuffix, 
    "libpng"..ReleaseSuffix, 
    "libtga"..ReleaseSuffix}

PostBuildStr="cd ../../../deploy_scripts\ncall deploy_runtime_libs.bat"