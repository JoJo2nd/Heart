
SlnName = "heart_tool"
BinType = "tools"
DebugSuffix = "_td"
ReleaseSuffix = "_tr"

dofile "../project_common/heart_common_proj.lua"
os.execute("\"..\\deploy_scripts\\deploy_external_libs.bat\"");
table.insert(HeartDefines,"HEART_TOOL_BUILD")

solution (SlnName)
    location (SlnOutput)
    configurations ({DebugCfgName, ReleaseCfgName})
    
    libdirs {LibDirs}
    
    configuration (DebugCfgName)
        targetsuffix(DebugSuffix)
        
    configuration (ReleaseCfgName)
        targetsuffix(ReleaseSuffix)
    
    dofile "heartcore.proj.lua" --1st project becomes the start up project
    dofile "zlib.proj.lua"
    dofile "lua.proj.lua"
    dofile "minizip.proj.lua"
    dofile "crypto.proj.lua"
    dofile "libvorbis.proj.lua"
    dofile "libvorbisfile.proj.lua"
    dofile "libogg.proj.lua"
    
    --- offline util libs
    dofile "libpng.proj.lua"
    dofile "libtga.proj.lua"
    
    -- resource loader projects
    dofile "texture_loader.proj.lua"
    dofile "font_loader.proj.lua"
    dofile "material_loader.proj.lua"
    dofile "shader_loader.proj.lua"
    dofile "mesh_loader.proj.lua"


