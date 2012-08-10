
SlnName = "ResourceLoaders"
BinType = "game"

dofile "HeartCommonProj.lua"

solution (SlnName)
    location (SlnOutput)
    configurations ({DebugCfgName, ReleaseCfgName})
    
    dofile "project_scripts/libpng.proj.lua"
    dofile "project_scripts/libtga.proj.lua"
    
    -- resoruce loader projects
    dofile "project_scripts/texture_loader.proj.lua"
    dofile "project_scripts/font_loader.proj.lua"
    dofile "project_scripts/material_loader.proj.lua"
    dofile "project_scripts/shader_loader.proj.lua"