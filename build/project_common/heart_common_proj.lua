
function ssub(str, args) 
    --print(str)
    s=string.gsub(str, "%$%(?(%w+)%)?", args)
    return s
end

function appendSuffixToTableEntries(t, sf)
    rt = {}
    for k,v in pairs(t) do
        rt[k] = v..sf
    end
    return rt
end

function printtable(t)
    for k,v in pairs(t) do
        print(k,v)
    end
end

table.splice = function (a,b)
    r={}
    for k,v in pairs(a) do
        r[k]=v
    end
    for k,v in pairs(b) do
        r[k]=v
    end
    return r
end

DXSDK_DIR=os.getenv("DXSDK_DIR")
VSTypeStr=os.getenv("VSTYPE")
HeartCommonVars={
    DXSDK=DXSDK_DIR,
    BUILDROOT=heartBuildRoot,
    REPOROOT=heartRepoRoot,
    BINTOOLROOT=heartBinToolRoot,
    BINGAMEROOT=heartBinGameRoot,
    BINROOT=heartBinRoot,
    BINTYPE=BinType,
    VSTYPE=VSTypeStr,
}

DebugCfgName = "Debug"
ReleaseCfgName = "Release"
SlnOutput=ssub("$(BUILDROOT)/$ACTION/$BINTYPE/$SLNNAME/", table.splice(HeartCommonVars,{ACTION=_ACTION,SLNNAME=SlnName}))
SlnDir=SlnOutput
ProjectDir = SlnDir.."../projects/"
TargetDir = SlnDir.."../lib/"
DebugDir=ssub("$BINROOT/$BINTYPE",HeartCommonVars)
ToolDebugDirEx="$BINTOOLROOT/$PROJECT/$CONFIG"


HeartPlatformIncludeDirs = {
    ssub("$DXSDK/include",HeartCommonVars)
}
HeartIncludeDirs = {
    HeartPlatformIncludeDirs,
    --common
    ssub("$REPOROOT/heart/include", HeartCommonVars),
    ssub("$REPOROOT/heart/include/pal/win32", HeartCommonVars),
    ssub("$REPOROOT/external/zlib/include", HeartCommonVars),
    ssub("$REPOROOT/external/lua/src", HeartCommonVars),
    ssub("$REPOROOT/external/libpng/include", HeartCommonVars),
    ssub("$REPOROOT/external/minizip/include", HeartCommonVars),
    ssub("$REPOROOT/external/crypto/include", HeartCommonVars),
    ssub("$REPOROOT/external/rapidxml-1.13/include", HeartCommonVars),
    ssub("$REPOROOT/external/libvorbis/include", HeartCommonVars),
    ssub("$REPOROOT/external/libogg/include", HeartCommonVars),
    ssub("$REPOROOT/external/libtga/include", HeartCommonVars),
    ssub("$REPOROOT/external/openal/include", HeartCommonVars)
}
wxWidgetsIncludeDirs = {
    ssub("$REPOROOT/external/wxWidgets-2.9.4/include/msvc", HeartCommonVars),
    ssub("$REPOROOT/external/wxWidgets-2.9.4/include", HeartCommonVars)
}

wxWidgetsDefines={
    "WXUSINGDLL",
    "wxMSVC_VERSION_AUTO"
}
HeartDefines= {
    "HEART_ENGINE",
    "HEART_PLAT_DX11",
    "USE_DL_PREFIX",
    "ONLY_MSPACES",
    "HEART_LUA_LIBS"
}
if BinType == "tools" then
    table.insert(HeartDefines, "HEART_TOOL_BUILD")
end
PlatformDefines={
    "WIN32",
    "_WIN32",
    "WINDOWS",
    "_WINDOWS"
}
CommonDefines= {
    {PlatformDefines},
    "_CRT_SECURE_NO_WARNINGS",
    "HEART_ENGINE_LIB"
}
SharedLibDefines = {
    "_DLL",
    "SHARED_LIB",
    "HEART_DLL"
}
DebugDefines={
    "_DEBUG",
    "DEBUG"
}
DebugOptions={
    "Symbols",
    "NoMinimalRebuild"
}
ReleaseDefines={
    "NDEBUG",
    "RELEASE"
}
ReleaseOptions={
    "Optimize",
    "NoEditAndContinue",
    "NoMinimalRebuild"
}

LibDirs={
    ssub("$REPOROOT/external/openal/libs/Win32", HeartCommonVars),
    ssub("$DXSDK/lib/x86", HeartCommonVars),
}

wxWidgetsLibsDirs={
    ssub("$REPOROOT/external/wxWidgets-2.9.4/lib/$(VSTYPE)0_dll", HeartCommonVars),
}

PlatformLibs={
    "d3d11",
    "d3dcompiler",
    "d3dx11",
    "dxguid",
    "xinput",
    "openAL32",
    "dbghelp"
}
HeartLibsDebug={
    "heart", 
    "crypto", 
    "lua",
    "zlib",
    "libpng", 
    "libtga",
}
HeartLibsRelease={
    "heart", 
    "crypto", 
    "lua", 
    "zlib",
    "libpng", 
    "libtga",
}

PostBuildDeployCmd=[[
cd $(REPOROOT)build/deploy_scripts
call deploy_lib.bat $LIBNAME "$TARGETDIR" "$BINGAMEROOT"
]]
PostBuildPluginDeployCmd=[[
cd $(REPOROOT)build/deploy_scripts
call deploy_lib.bat $LIBNAME "$TARGETDIR" "$BINGAMEROOT/PLUGIN"
]]
PostBuildToolDeployCmd=[[
cd $(REPOROOT)build/deploy_scripts
call deploy_lib.bat $LIBNAME "$TARGETDIR" "$BINTOOLROOT/$PROJECT/$CONFIG"
]]
PostBuildToolPluginDeployCmd=[[
cd $(REPOROOT)build/deploy_scripts
call deploy_lib.bat $LIBNAME "$TARGETDIR" "$BINTOOLROOT/plugin/$CONFIG"
]]
DeployExternalLibCmd=[[
cd $(REPOROOT)build/deploy_scripts
call deploy_external_libs.bat "$TARGETDIR"
]]
DeploywxWidgetsCmd=[[
cd $(REPOROOT)build/deploy_scripts
call deploy_wxwidgets_libs.bat $WXTYPE "$BINTOOLROOT/$PROJECT/$CONFIG"
]]

