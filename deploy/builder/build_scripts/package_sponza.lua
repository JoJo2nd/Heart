local infiles=nil
local depfiles=nil
local progparams=nil
local materialpath=nil
local packagename="SPONZA"
--
--
--
materials = {
    {path="src://materials/sponza/sponza_basic.mfx", res="DEFAULTMATERIAL"},
    {path="src://materials/sponza/arch.mfx", res="ARCH"},
    {path="src://materials/sponza/background.mfx", res="BACKGROUND"},
    {path="src://materials/sponza/bricks.mfx", res="BRICKS"},
    {path="src://materials/sponza/chain.mfx", res="CHAIN"},
    {path="src://materials/sponza/ceiling.mfx", res="CEILING"},
    {path="src://materials/sponza/column_a.mfx", res="COLUMN_A"},
    {path="src://materials/sponza/column_b.mfx", res="COLUMN_B"},
    {path="src://materials/sponza/column_c.mfx", res="COLUMN_C"},
    {path="src://materials/sponza/details.mfx", res="DETAILS"},
    {path="src://materials/sponza/fabric_a.mfx", res="FABRIC_A"},
    {path="src://materials/sponza/fabric_c.mfx", res="FABRIC_C"},
    {path="src://materials/sponza/fabric_d.mfx", res="FABRIC_D"},
    {path="src://materials/sponza/fabric_f.mfx", res="FABRIC_F"},
    {path="src://materials/sponza/fabric_g.mfx", res="FABRIC_G"},
    {path="src://materials/sponza/fabric_e.mfx", res="FABRIC_E"},
    {path="src://materials/sponza/flagpole.mfx", res="FLAGPOLE"},
    {path="src://materials/sponza/floor.mfx", res="FLOOR"},
    {path="src://materials/sponza/gi_flag.mfx", res="GI_FLAG"},
    {path="src://materials/sponza/leaf.mfx", res="LEAF"},
    {path="src://materials/sponza/lion.mfx", res="LION"},
    {path="src://materials/sponza/sponza_basic.mfx", res="SPONZA_BASIC"},
    {path="src://materials/sponza/roof.mfx", res="ROOF"},
    {path="src://materials/sponza/vase.mfx", res="VASE"},
    {path="src://materials/sponza/vase_hanging.mfx", res="VASE_HANGING"},
    {path="src://materials/sponza/vase_plant.mfx", res="VASE_PLANT"},
    {path="src://materials/sponza/vase_round.mfx", res="VASE_ROUND"},
}
for k, mat in pairs(materials) do 
    databuilder.addresource{
        package=packagename,
        resname=mat.res.."_M",
        restype="mat_fx",
        inputfiles={mat.path},
        depfiles=material.scanincludes(mat.path)
    }
end
--
--
--
shaders={
    {path="src://shaders/gbuffer.c", res="GBUFFER_VS", params={entry="vertexMain", profile="vs_5_0", warningsaserrors=true}},
    {path="src://shaders/gbuffer.c", res="GBUFFER_PS", params={entry="pixelMain", profile="ps_5_0", warningsaserrors=true}},
    {path="src://shaders/gbuffer.c", res="GBUFFER_NO_NRM_VS", params={entry="vertexMain", profile="vs_5_0", defines={NO_NORMAL_MAP="1"}, warningsaserrors=true}},
    {path="src://shaders/gbuffer.c", res="GBUFFER_NO_NRM_PS", params={entry="pixelMain", profile="ps_5_0", defines={NO_NORMAL_MAP="1"}, warningsaserrors=true}},
}
for k, gpu in pairs(shaders) do 
    databuilder.addresource{
        package=packagename,
        resname=gpu.res,
        restype="gpu_prog",
        inputfiles={gpu.path},
        depfiles=gpuprog.preprocess(gpu.path, gpu.params),
        parameters=gpu.params
    }
end
--
--
--

-- textures = {
    -- CHECKER = {input="src://textures/convert/default.dds", sRGB=true}
-- }
-- for k, tex in pairs(textures) do
    -- databuilder.addresource{
        -- package="SPONZA",
        -- resname=k,
        -- restype="texture",
        -- inputfiles={ tex.input },
        -- parameters={ sRGB=tex.sRGB },
    -- }
-- end
--
--
--
databuilder.addresource{
    package="SPONZA",
    resname="SPONZA",
    restype="mesh",
    inputfiles={"src://mesh/sponza.mdf"},
}

