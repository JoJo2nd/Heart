local infiles=nil
local depfiles=nil
local progparams=nil
local materialpath=nil

--
-- package: INSTANCETEST
--
databuilder.addresource{
    package="INSTANCETEST",
    resname="BOCO",
    restype="mesh",
    inputfiles={"src://mesh/boco_inst.mdf"}
}
materialpath="src://materials/boco_mat_inst.mfx"
databuilder.addresource{
    package="INSTANCETEST",
    resname="BOCOINSTMAT",
    restype="mat_fx",
    inputfiles={materialpath,},
    depfiles=material.scanincludes(materialpath),
}
infiles={"src://shaders/boco_inst.c"}
progparams={
    entry="mainFP",
    profile="ps_4_0",
    defines={},
}
depfiles = gpuprog.preprocess("src://shaders/boco_inst.c", progparams)
databuilder.addresource{
    package="INSTANCETEST",
    resname="BOCO_FP",
    restype="gpu_prog",
    inputfiles=infiles,
    depfiles=depfiles,
    parameters=progparams
}
infiles={"src://shaders/boco_inst.c"}
progparams={
    entry="mainInstVP",
    profile="vs_4_0",
    defines={},
}
depfiles = gpuprog.preprocess("src://shaders/boco_inst.c", progparams)
databuilder.addresource{
    package="INSTANCETEST",
    resname="BOCO_VP",
    restype="gpu_prog",
    inputfiles=infiles,
    depfiles=depfiles,
    parameters=progparams
}