require "util"

function rendererResize(width, height)
    print("Lua resize call ", width, "x", height)
    heart.resizeGlobalTexture("back_buffer", width, height);
    heart.resizeGlobalTexture("depth_buffer", width, height);
    heart.resizeGlobalTexture("z_pre_pass", width, height);
    heart.resizeGlobalTexture("gbuffer_albedo", width, height);
    heart.resizeGlobalTexture("gbuffer_normal", width, height);
    heart.resizeGlobalTexture("gbuffer_spec", width, height);
    heart.resizeGlobalTexture("blur_target", width, height);
    heart.resizeGlobalTexture("blur_rw_texture", width, height);
end

function registerGlobalTextures()
    local rndrWidth,rndrHeight=heart.getWindowWidthHeight()

    heart.registerGlobalTexture {
        name="back_buffer",
        aliases={
            "back_buffer",
            "g_back_buffer",
        },
        width=rndrWidth,
        height=rndrHeight,
        format="rgba8_srgb_unorm",
        rendertarget=true,
    }
    heart.registerGlobalTexture {
        name="gbuffer_albedo",
        aliases={
            "gbuffer_albedo",
            "galbedo"
        },
        width=rndrWidth,
        height=rndrHeight,
        format="rgba16_float",
        rendertarget=true,
    }
    heart.registerGlobalTexture {
        name="gbuffer_normal",
        aliases={
            "gbuffer_normal",
            "gnormal"
        },
        width=rndrWidth,
        height=rndrHeight,
        format="rgba16_float",
        rendertarget=true,
    }
    heart.registerGlobalTexture {
        name="gbuffer_spec",
        aliases={
            "gbuffer_spec",
            "gspec"
        },
        width=rndrWidth,
        height=rndrHeight,
        format="rgba16_float",
        rendertarget=true,
    }
    heart.registerGlobalTexture {
        name="depth_buffer",
        aliases = {
            "depth_buffer",
            "g_depth_buffer",
            "z_buffer",
            "g_z_buffer",
        },
        width=rndrWidth,
        height=rndrHeight,
        format="r32_typeless",
        depth=true,
    }
    heart.registerGlobalTexture {
        name="z_pre_pass",
        aliases = {
            "g_zPrePass",
            "g_zprepass",
            "ZPrePass",
            "zprepass",
        },
        width=rndrWidth,
        height=rndrHeight,
        format="r32_typeless",
        depth=true,
    }
    heart.registerGlobalTexture {
        name="blur_target",
        aliases={
            "blur_target",
            "g_blur_target",
        },
        width=rndrWidth,
        height=rndrHeight,
        format="rgba16_float",
        rendertarget=true,
    }
    heart.registerGlobalTexture {
        name="blur_rw_texture",
        aliases={
            "blur_rw_texture",
        },
        width=rndrWidth,
        height=rndrWidth,
        format="r32_uint",
        uav=true,
    }
    heart.registerWindowResizeCallback(rendererResize)
end

registerGlobalTextures()

-- can be written like
-- camera_block = { ... }
-- heart.registerParameterBlock(camera_block)
-- camera_block = nil
-- or
-- local camera_block = { ... }
-- heart.registerParameterBlock(camera_block)

heart.registerRenderTechnique("zprepass")
heart.registerRenderTechnique("postzmain")
heart.registerRenderTechnique("main")
heart.registerRenderTechnique("lighting")

-- heart.registerParameterBlock {
    -- name="FrameConstants",
    -- aliases={
        -- "FrameConstants"
    -- },
    -- parameters={
        -- {
            -- name="g_time",
            -- size=4,
        -- },
        -- {
            -- name="g_fracTime",
            -- size=4,
        -- },
    -- },
-- }

heart.registerParameterBlock {
    name = "CameraConstants", -- Name the game uses to find/set this block
    aliases = { -- List of names that this file links too in shaders
        "ViewportConstants",
        "camera_constants",
        "CameraConstants",
    },
    parameters = { -- Parameters in this block are assumed to be in the same order in the shaders
        {
            name = "g_View", -- var name in const block
            size = 64, -- size of var in bytes
            initial = 0, -- what to set the data too, if only one entire var is set to this
        },
        { 
            name = "g_ViewInverse",
            size = 64,
            initial = { -- what to set the data too, can use list
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1,
            },
        },
        {
            name = "g_ViewInverseTranspose",
            size = 64,
            -- no initial member leaves the data uninitialised
        },
        {
            name = "g_Projection",
            size = 64,
        },
        {
            name = "g_ProjectionInverse",
            size = 64,
        },
        {
            name = "g_ViewProjection",
            size = 64,
        },
        { 
            name = "g_ViewProjectionInverse",
            size = 64,
        },
        { 
            name = "g_viewportSize",
            size = 16,
        },
    },
}

heart.registerParameterBlock {
    name="InstanceConstants",
    aliases={
        "InstanceConstants",
    },
    parameters={
        {
            name="g_World",
            size=64,
        },
    },
}