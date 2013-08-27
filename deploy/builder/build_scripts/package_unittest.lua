local infiles=nil
local depfiles=nil
local progparams=nil
local materialpath=nil

--
-- package: UNITTEST
--
databuilder.addresource{
    package="UNITTEST",
    resname="BOCO",
    restype="mesh",
    inputfiles={"src://mesh/boco.mdf"},
}
infiles={"src://shaders/boco_inst.hlsl"}
progparams={
    entry="mainFP",
    profile="ps_4_0",
    defines={},
}
depfiles = gpuprog.preprocess("src://shaders/boco_inst.hlsl", progparams)
databuilder.addresource{
    package="UNITTEST",
    resname="BOCO_FP",
    restype="gpu_prog",
    inputfiles=infiles,
    depfiles=depfiles,
    parameters=progparams
}
infiles={"src://shaders/boco_inst.hlsl"}
progparams={
    entry="mainVP",
    profile="vs_4_0",
    defines={},
}
depfiles = gpuprog.preprocess("src://shaders/boco_inst.hlsl", progparams)
databuilder.addresource{
    package="UNITTEST",
    resname="BOCO_VP",
    restype="gpu_prog",
    inputfiles=infiles,
    depfiles=depfiles,
    parameters=progparams
}
materialpath="src://materials/boco_mat.mfx"
databuilder.addresource{
    package="UNITTEST",
    resname="BOCO_MAT",
    restype="mat_fx",
    inputfiles={materialpath,},
    depfiles=material.scanincludes(materialpath),
}
--databuilder.addresource{
--    package="UNITTEST",
--    resname="DEBUG_FONT",
--    restype="font",
--    inputfiles={"src://fonts/debug/debug.fnt"},
--    parameters={
--        headeroutput="src://debug_font_data.hlsl"
--    },
--}
databuilder.addresource{
    package="UNITTEST",
    resname="DEBUG_FONT_PAGE",
    restype="texture",
    inputfiles={"src://fonts/debug/debug_0.dds"},
    parameters={
        sRGB=false
    },
}