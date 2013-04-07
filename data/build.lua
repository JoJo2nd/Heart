databuilder = require "databuilder"fs = require "filesystem"gpuprog = require "gpuprogram"print("Starting custom build script")databuilder.sourcerootpath("C:/dev/heart_lua/data")--fs.currentpath())databuilder.destrootpath(string.format("%s/deploy/game/GAMEDATA","C:/dev/heart_lua"))--fs.currentpath()))databuilder.addbuilder {    typename="texture",    buildfunc= function (inputfiles, depfiles, params)        print("building texture...")        return {}    end,}databuilder.addbuilder {    typename="gpu_prog",    buildfunc= function (inputfiles, depfiles, params, path)        print("building GPU program...")        return gpuprog.compile(inputfiles, depfiles, params, path)    end,}databuilder.addbuilder {    typename="mat_fx",    buildfunc= function (inputfiles, depfiles, params)        print("building material FX...")    end,}databuilder.addresource{    package="TESTTEXTURE",    resname="TEXTURE1",    restype="texture",    inputfiles={        "src://textures/naruto.dds"    },}-- Materials & Shaders Util varslocal infiles=nillocal depfiles=nillocal progparams=nil-- Common Materials & Shadersinfiles={"src://shaders/sibenik.c"}progparams={    entry="mainColFP",    profile="ps_4_0",    defines={        TESTDEFINE="1",    },}depfiles = gpuprog.preprocess("src://shaders/sibenik.c", progparams)databuilder.addresource{    package="MATERIALS",    resname="BLINNPHONGCOL_FP",    restype="gpu_prog",    inputfiles=infiles,    depfiles=depfiles,    parameters=progparams}--infiles={"src://shaders/sibenik.c"}progparams.entry="mainColVP"progparams.profile="vs_4_0"defines={}depfiles = gpuprog.preprocess("src://shaders/sibenik.c", progparams)databuilder.addresource{    package="MATERIALS",    resname="BLINNPHONGCOL_VP",    restype="gpu_prog",    inputfiles=infiles,    depfiles=depfiles,    parameters=progparams}--infiles={"src://shaders/sibenik.c"}progparams.entry="mainFP"progparams.profile="ps_4_0"defines={}depfiles = gpuprog.preprocess("src://shaders/sibenik.c", progparams)databuilder.addresource{    package="MATERIALS",    resname="BLINNPHONG_FP",    restype="gpu_prog",    inputfiles=infiles,    depfiles=depfiles,    parameters=progparams}--infiles={"src://shaders/sibenik.c"}progparams.entry="mainVP"progparams.profile="vs_4_0"defines={}depfiles = gpuprog.preprocess("src://shaders/sibenik.c", progparams)databuilder.addresource{    package="MATERIALS",    resname="BLINNPHONG_VP",    restype="gpu_prog",    inputfiles=infiles,    depfiles=depfiles,    parameters=progparams}--infiles={"src://shaders/minecraft_style.c"}progparams.entry="mainAlphaCutoutFP"progparams.profile="ps_4_0"defines={}depfiles = gpuprog.preprocess("src://shaders/minecraft_style.c", progparams)databuilder.addresource{    package="MATERIALS",    resname="MINEALPHATESTFP",    restype="gpu_prog",    inputfiles=infiles,    depfiles=depfiles,    parameters=progparams}--infiles={"src://shaders/minecraft_style.c"}progparams.entry="mainFP"progparams.profile="ps_4_0"defines={}depfiles = gpuprog.preprocess("src://shaders/minecraft_style.c", progparams)databuilder.addresource{    package="MATERIALS",    resname="MINEFP",    restype="gpu_prog",    inputfiles=infiles,    depfiles=depfiles,    parameters=progparams}--infiles={"src://shaders/minecraft_style.c"}progparams.entry="mainVP"progparams.profile="vs_4_0"defines={}depfiles = gpuprog.preprocess("src://shaders/minecraft_style.c", progparams)databuilder.addresource{    package="MATERIALS",    resname="MINEVP",    restype="gpu_prog",    inputfiles=infiles,    depfiles=depfiles,    parameters=progparams}--infiles={"src://shaders/perlin.c"}progparams.entry="csPerlinNoise"progparams.profile="cs_5_0"defines={}depfiles = gpuprog.preprocess("src://shaders/perlin.c", progparams)databuilder.addresource{    package="MATERIALS",    resname="PERLIN_CS",    restype="gpu_prog",    inputfiles=infiles,    depfiles=depfiles,    parameters=progparams}--infiles={"src://shaders/blur_blit.c"}progparams.entry="mainVP"progparams.profile="vs_5_0"defines={}depfiles = gpuprog.preprocess("src://shaders/blur_blit.c", progparams)databuilder.addresource{    package="MATERIALS",    resname="BLUR_BLIT_VP",    restype="gpu_prog",    inputfiles=infiles,    depfiles=depfiles,    parameters=progparams}--infiles={"src://shaders/blur_blit.c"}progparams.entry="BlitColor_PS"progparams.profile="ps_5_0"defines={}depfiles = gpuprog.preprocess("src://shaders/blur_blit.c", progparams)databuilder.addresource{    package="MATERIALS",    resname="BLUR_BLIT_FP",    restype="gpu_prog",    inputfiles=infiles,    depfiles=depfiles,    parameters=progparams}------databuilder.addresource{    package="MATERIALS",    resname="MINEMAT",    restype="mat_fx",    inputfiles={"src://materials/minemat.mfx",}}databuilder.addresource{    package="MATERIALS",    resname="MINEMAT_CUTOUT",    restype="mat_fx",    inputfiles={"src://materials/minemat_cutout.mfx",}}databuilder.addresource{    package="MATERIALS",    resname="MINEMAT_TRANS",    restype="mat_fx",    inputfiles={"src://materials/minemat_trans.mfx",}}