databuilder = require "databuilder"fs = require "filesystem"gpuprog = require "gpuprogram"texture = require "texture"material = require "material"mesh = require "mesh"font = require "font"xml=require "LuaXml"print("Starting custom build script")-- Materials & Shaders Util varslocal infiles=nillocal depfiles=nillocal progparams=nillocal materialpath=nildatabuilder.sourcerootpath("C:/dev/heart_lua/data")--fs.currentpath())databuilder.destrootpath(string.format("%s/deploy/game/GAMEDATA","C:/dev/heart_lua"))--fs.currentpath()))function readTextureDataStore(path)    texxml = xml.load(path)    textures = texxml:find("textures")    if textures ~= nil then        for k,v in pairs(textures) do            if v.tag ~= nill and v:tag() == "texture" and v["package"] ~= "" and v["resource"] ~= "" then                databuilder.addresource{                    package=v["package"],                    resname=v["resource"],                    restype="texture",                    inputfiles={v["outputfile"]},                    depfiles={},                    flags= { sRGB=v["gammacorrect"]}                }            end        end    endenddatabuilder.addbuilder {    typename="texture",    buildfunc= function (inputfiles, depfiles, params, path)        return texture.compile(inputfiles, depfiles, params, path)    end,}databuilder.addbuilder {    typename="gpu_prog",    buildfunc= function (inputfiles, depfiles, params, path)        return gpuprog.compile(inputfiles, depfiles, params, path)    end,    parameters = {        debug=true,        skipoptimization=false,        warningsaserrors=false,        includesource=true,    }}databuilder.addbuilder {    typename="mat_fx",    buildfunc= function (inputfiles, depfiles, params, path)        return material.compile(inputfiles, depfiles, params, path)    end,}databuilder.addbuilder {    typename="mesh",    buildfunc= function (inputfiles, depfiles, params, path)        return mesh.compile(inputfiles, depfiles, params, path)    end,}databuilder.addbuilder {    typename="font",    buildfunc= function (inputfiles, depfiles, params, path)        return font.compile(inputfiles, depfiles, params, path)    end,}readTextureDataStore("C:/dev/heart_lua/data/texture_data_store.xml")dofile("build_scripts/package_unittest.lua")dofile("build_scripts/package_materials.lua")dofile("build_scripts/package_sibenik.lua")dofile("build_scripts/package_instancetest.lua")dofile("build_scripts/package_complexmesh1.lua")dofile("build_scripts/package_complexmesh2.lua")dofile("build_scripts/package_sponza.lua")