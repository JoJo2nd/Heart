
DebugCfgName = "Debug"
ReleaseCfgName = "Release"
SlnOutput = "../built_projects/".._ACTION.."/"..BinType.."/"..SlnName.."/"
SlnDir = SlnOutput
ProjectDir = SlnDir.."../projects/"
TargetDir = SlnDir.."../lib/"
DebugDir="../../bin/"..BinType
HeartPlatformIncludeDirs = {
    os.getenv("DXSDK_DIR").."/Include",
}
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
    "../../external/openal/include",
}
wxWidgetsIncludeDirs = {
    "../../external/wxWidgets-2.9.2/include/msvc",
    "../../external/wxWidgets-2.9.2/include",
}

HeartDefines= {"HEART_ENGINE","HEART_PLAT_DX11","USE_DL_PREFIX","ONLY_MSPACES","HEART_LUA_LIBS"}
if BinType == "tools" then
    table.insert(HeartDefines, "HEART_TOOL_BUILD")
end
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
wxWidgetsLibsDirs={
    "../../external/wxWidgets-2.9.2/lib/vc_lib",
}
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
    PostBuildStrPlugin="cd ../../../../deploy_scripts\ncall deploy_plugin_lib.bat "
else
    PostBuildStr="cd ../../../../deploy_scripts\ncall deploy_tool.bat "
end

function appendSuffixToTableEntries(t, sf)
    rt = {}
    for k,v in pairs(t) do
        rt[k] = v..sf
    end
    return rt
end
