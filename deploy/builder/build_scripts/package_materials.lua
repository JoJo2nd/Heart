local infiles=nil
local depfiles=nil
local progparams=nil
local materialpath=nil

--
-- MATERIALS
--
databuilder.addresource{
    package="MATERIALS",
    resname="NARUTO_TEST",
    restype="texture",
    inputfiles={
        "src://textures/naruto_test.dds"
    },
    parameters={
        sRGB=true
    },
}
databuilder.addresource{
    package="MATERIALS",
    resname="LIGHT",
    restype="texture",
    inputfiles={
        "src://textures/light.dds"
    },
    parameters={
        sRGB=true
    },
}
databuilder.addresource{
    package="MATERIALS",
    resname="MINE",
    restype="texture",
    inputfiles={
        "src://textures/mine.dds"
    },
    parameters={
        sRGB=true
    },
}

-- Common Materials & Shaders
infiles={"src://shaders/sibenik.c"}
progparams={
    entry="mainColFP",
    profile="ps_4_0",
    defines={
        TESTDEFINE="1",
    },
}
depfiles = gpuprog.preprocess("src://shaders/sibenik.c", progparams)
databuilder.addresource{
    package="MATERIALS",
    resname="BLINNPHONGCOL_FP",
    restype="gpu_prog",
    inputfiles=infiles,
    depfiles=depfiles,
    parameters=progparams
}
--
infiles={"src://shaders/sibenik.c"}
progparams.entry="mainColVP"
progparams.profile="vs_4_0"
defines={}
depfiles = gpuprog.preprocess("src://shaders/sibenik.c", progparams)
databuilder.addresource{
    package="MATERIALS",
    resname="BLINNPHONGCOL_VP",
    restype="gpu_prog",
    inputfiles=infiles,
    depfiles=depfiles,
    parameters=progparams
}
--
infiles={"src://shaders/sibenik.c"}
progparams.entry="mainFP"
progparams.profile="ps_4_0"
defines={}
depfiles = gpuprog.preprocess("src://shaders/sibenik.c", progparams)
databuilder.addresource{
    package="MATERIALS",
    resname="BLINNPHONG_FP",
    restype="gpu_prog",
    inputfiles=infiles,
    depfiles=depfiles,
    parameters=progparams
}
--
infiles={"src://shaders/sibenik.c"}
progparams.entry="mainVP"
progparams.profile="vs_4_0"
defines={}
depfiles = gpuprog.preprocess("src://shaders/sibenik.c", progparams)
databuilder.addresource{
    package="MATERIALS",
    resname="BLINNPHONG_VP",
    restype="gpu_prog",
    inputfiles=infiles,
    depfiles=depfiles,
    parameters=progparams
}
--
infiles={"src://shaders/minecraft_style.c"}
progparams.entry="mainAlphaCutoutFP"
progparams.profile="ps_4_0"
defines={}
depfiles = gpuprog.preprocess("src://shaders/minecraft_style.c", progparams)
databuilder.addresource{
    package="MATERIALS",
    resname="MINEALPHATESTFP",
    restype="gpu_prog",
    inputfiles=infiles,
    depfiles=depfiles,
    parameters=progparams
}
--
infiles={"src://shaders/minecraft_style.c"}
progparams.entry="mainFP"
progparams.profile="ps_4_0"
defines={}
depfiles = gpuprog.preprocess("src://shaders/minecraft_style.c", progparams)
databuilder.addresource{
    package="MATERIALS",
    resname="MINEFP",
    restype="gpu_prog",
    inputfiles=infiles,
    depfiles=depfiles,
    parameters=progparams
}
--
infiles={"src://shaders/minecraft_style.c"}
progparams.entry="mainVP"
progparams.profile="vs_4_0"
defines={}
depfiles = gpuprog.preprocess("src://shaders/minecraft_style.c", progparams)
databuilder.addresource{
    package="MATERIALS",
    resname="MINEVP",
    restype="gpu_prog",
    inputfiles=infiles,
    depfiles=depfiles,
    parameters=progparams
}
--
infiles={"src://shaders/perlin.c"}
progparams.entry="csPerlinNoise"
progparams.profile="cs_5_0"
defines={}
depfiles = gpuprog.preprocess("src://shaders/perlin.c", progparams)
databuilder.addresource{
    package="MATERIALS",
    resname="PERLIN_CS",
    restype="gpu_prog",
    inputfiles=infiles,
    depfiles=depfiles,
    parameters=progparams
}
--
infiles={"src://shaders/blur_blit.c"}
progparams.entry="mainVP"
progparams.profile="vs_5_0"
defines={}
depfiles = gpuprog.preprocess("src://shaders/blur_blit.c", progparams)
databuilder.addresource{
    package="MATERIALS",
    resname="BLUR_BLIT_VP",
    restype="gpu_prog",
    inputfiles=infiles,
    depfiles=depfiles,
    parameters=progparams
}
--
infiles={"src://shaders/blur_blit.c"}
progparams.entry="BlitColor_PS"
progparams.profile="ps_5_0"
defines={}
depfiles = gpuprog.preprocess("src://shaders/blur_blit.c", progparams)
databuilder.addresource{
    package="MATERIALS",
    resname="BLUR_BLIT_FP",
    restype="gpu_prog",
    inputfiles=infiles,
    depfiles=depfiles,
    parameters=progparams
}
--
--
--
materialpath="src://materials/minemat.mfx"
databuilder.addresource{
    package="MATERIALS",
    resname="MINEMAT",
    restype="mat_fx",
    inputfiles={materialpath,},
    depfiles=material.scanincludes(materialpath),
}
materialpath="src://materials/minemat_cutout.mfx"
databuilder.addresource{
    package="MATERIALS",
    resname="MINEMAT_CUTOUT",
    restype="mat_fx",
    inputfiles={materialpath,},
    depfiles=material.scanincludes(materialpath),
}
materialpath="src://materials/minemat_trans.mfx"
databuilder.addresource{
    package="MATERIALS",
    resname="MINEMAT_TRANS",
    restype="mat_fx",
    inputfiles={materialpath,},
    depfiles=material.scanincludes(materialpath),
}