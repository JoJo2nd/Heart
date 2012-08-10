
SlnName = "NewTestBed"
BinType = "game"

dofile "HeartCommonProj.lua"

solution (SlnName)
    location (SlnOutput)
    configurations ({DebugCfgName, ReleaseCfgName})
    
    dofile "project_scripts/bootloader.proj.lua" --1st project becomes the start up project
    dofile "project_scripts/zlib.proj.lua"
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

