local M = {}
local G = _G
-- pull in local modules
local filesystem = require "filesystem"
local string = string
local io = io
local os = os
local strupr = string.upper
local strfmt = string.format
local strmatch = string.match
local strsub = string.gsub
local loadfilechunk = loadfile
local ipairs = ipairs
local pairs=pairs
local type=type
local error=error
local pcall=pcall
local filemodtime = filesystem.modifieddate
-- disable access to globals
_ENV=nil

--local internal vars
local resources_={}
local builders_={}
local sourceRootDir_=""
local destRootDir_=""
local logfile=nil
local logerr=nil

logfile, logerr=io.open(strfmt("build_log_%s.txt",os.date("%Y-%m-%d_%H-%M")), "w")

local function addBuilder(boptions) 
    if (type(boptions.typename) ~= "string") then
        error("builder has incorrect typename")
    elseif (type(boptions.buildfunc) ~= "function") then
        error("builder has not builder function")
    end
    builders_[boptions.typename]=boptions.buildfunc
end

local function addResource(resopt)
    if type(resopt.package) ~= "string" then
        error("resource option error: package is not a string")
    elseif type(resopt.resname) ~= "string" then
        error("resource option error: resname is not a string")
    elseif type(resopt.restype) ~= "string" then
        error("resource option error: restype is not a string")
    elseif type(resopt.inputfiles) ~= "table" then
        error("resource option error: inputfiles are not a table or are an empty table")
    end
    local fullresname=strfmt("%s.%s", strupr(resopt.package), strupr(resopt.resname))
    if resources_[fullresname] ~= nil then 
        G.print(strfmt("Resource %s already exists, replacing", fullresname))
    else
        G.print(strfmt("Resource %s added", fullresname))
    end
    local inputpaths = {}
    local deppaths = {}
    local params = {}
    for k, i in pairs(resopt.inputfiles) do
        inputpaths[k]=strsub(i, "src://", sourceRootDir_)
    end
    if resopt.parameters ~= nil then
        for k, i in pairs(resopt.parameters) do
            params[k]=resopt.parameters[k]
        end
    end
    if resopt.depfiles ~= nil then
        for k, i in pairs(resopt.depfiles) do
            deppaths[k]=strsub(i, "src://", sourceRootDir_)
        end
    end
    resources_[fullresname] = {
        respath=fullresname,
        package=resopt.package,
        name=resopt.resname,
        restype=resopt.restype,
        inputfiles=inputpaths,
        depfiles=deppaths,
        parameters=params
    }
end

local function resetDataBuild()
    resources_ = {}
    builders_ = {}
    sourceRootDir_=nil
    destRootDir_=nil
end

local function getSourceRootPath(v)
    if v ~= nil then
        sourceRootDir_=v.."/"
		G.print(strfmt("Resource source path set to -> %s", sourceRootDir_))
    end
    return sourceRootDir_
end

local function getDestRootPath(v)
    if v ~= nil then
        destRootDir_=v.."/"
		G.print(strfmt("Package dest path set to -> %s", destRootDir_))
    end
    return destRootDir_
end

local function generateResourcesForBuild(matchopt)
    local resources={}
    local function matchmod(r)
        for _, file in r.inputfiles do
            if (matchopt.mod > filemodtime(file)) then
                return true
            end 
        end
        for _, file in r.depfiles do
            if (matchopt.mod > filemodtime(file)) then
                return true
            end 
        end
        return false
    end
    local function matchname(r) 
        if strmatch(r.respath, matchopt.namematch) == r.respath then 
            return true 
        else
            return false
        end
    end
    local function matchrestype(r)
        if restype == matchopt.type then
            return true
        else
            return false
        end
    end
    local function matchfilepath(r)
        for _, file in r.inputfiles do
            if strmatch(file, r.filematch) == file then
                return true
            end 
        end
        for _, file in r.depfiles do
            if strmatch(file, r.filematch) == file then
                return true
            end 
        end
        return false
    end
    for _, res in pairs(resources_) do
        if (matchopt.namematch ~= nil and matchname(res)) then
            resources[res.respath]=res
			G.print(strfmt("Adding resource %s for building", res.respath))
        elseif (matchopt.mod ~= nil and matchmod(res)) then
            resources[res.respath]=res
			G.print(strfmt("Adding resource %s for building", res.respath))
        elseif (matchopt.type ~= nil and matchrestype(res)) then
            resources[res.respath]=res
			G.print(strfmt("Adding resource %s for building", res.respath))
        elseif (matchopt.filematch ~= nil and matchfilepath(res)) then
            resources[res.respath]=res
			G.print(strfmt("Adding resource %s for building", res.respath))
        end
    end
	G.print("Found "..#resources.." resource(s) to build.")
    return resources
end

local function buildResources(matchopts, scriptpath)
    --create build log
    local oldprint=G.print
    G.print = function (...) 
        for i, v in ipairs{...} do
            logfile:write(G.tostring(v))
        end
        logfile:write("\n")
        oldprint(...)
    end 
    G.buildpathresolve = function (x) 
        return strsub(x, "(%w+)://", {src=sourceRootDir_, dest=destRootDir_})
    end
    local buildscript=nil
    local errmsg=nil
    buildscript, errmsg = loadfilechunk(scriptpath, "tb", G)
    if buildscript==nil then
        G.print(strfmt("Build script error: %s", errmsg))
        G.print=oldprint
        G.buildpathresolve=nil
        return
    end
    resetDataBuild()
	local runok
    runok, errmsg = pcall(buildscript) -- run build script
	if not runok then
		resetDataBuild()
		G.print(strfmt("Build script error: %s", errmsg))
        G.print=oldprint
        G.buildpathresolve=nil
        return
	end

    local resourcestobuild=generateResourcesForBuild(matchopts)

	for k, v in pairs(resourcestobuild) do
        local packdir="dest://"..v.package.."/"
        local fullrespath=packdir..v.name
        local realpath=G.buildpathresolve(packdir)
        G.print(strfmt("Creating directory %s->%s", packdir, realpath))
        filesystem.makedirectories(realpath)
		G.print(strfmt("Building %s resource %s to %s...", v.restype, v.respath, fullrespath))
		local depres = builders_[v.restype](v.inputfiles, v.depfiles, v.parameters, fullrespath)
	end

    G.print=oldprint
    G.buildpathresolve=nil
end

_ENV=G

return { -- Module functions
    reset           =resetDataBuild,
    sourcerootpath	=getSourceRootPath,
    destrootpath	=getDestRootPath,
    buildresources	=buildResources,
    addresource		=addResource,
    addbuilder		=addBuilder,
}


