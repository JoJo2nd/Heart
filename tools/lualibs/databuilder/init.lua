local M = {}
local G = _G
-- pull in local modules
local filesystem = require "filesystem"
local xml = require "LuaXml"
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
local globaltypeoptions_={}
local packages_={}
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
    if type(boptions.parameters) == "table" then 
        globaltypeoptions_[boptions.typename] = boptions.parameters
    else
        globaltypeoptions_[boptions.typename] = {}
    end
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
        inputpaths[k]=i
    end
    if resopt.parameters ~= nil then
        for k, i in pairs(resopt.parameters) do
            params[k]=resopt.parameters[k]
        end
    end
    if resopt.depfiles ~= nil then
        for k, i in pairs(resopt.depfiles) do
            deppaths[k]=i
        end
    end
    resources_[fullresname] = {
        respath=fullresname,
        package=resopt.package,
        name=resopt.resname,
        restype=resopt.restype,
        inputfiles=inputpaths,
        depfiles=deppaths,
        parameters=params,
        dependentpackages={}
    }
    if packages_[resopt.package] == nil then
        packages_[resopt.package] = {}
        packages_[resopt.package].resources = {}
    end
    G.table.insert(packages_[resopt.package].resources, resources_[fullresname])
end

local function resetDataBuild()
    resources_ = {}
    builders_ = {}
    packages_ = {}
    globaltypeoptions_ = {}
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
	G.print("Finished looking for resources to build.")
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
    local packagestouched={}

	for k, v in pairs(resourcestobuild) do
        local packdir="dest://"..v.package.."/"
        local fullrespath=packdir..v.name
        local realpath=G.buildpathresolve(packdir)
        local localparameters={}
        G.print(strfmt("Creating directory %s -> %s", packdir, realpath))
        filesystem.makedirectories(realpath)
		G.print(strfmt("Building \"%s\" resource %s to %s ", v.restype, v.respath, fullrespath))
        --copy parameters, global first then resource ones over write
        for paramname, param in pairs(globaltypeoptions_[v.restype]) do
            localparameters[paramname] = param
        end
        for paramname, param in pairs(v.parameters) do
            localparameters[paramname] = param
        end
        --
        v.dependentpackages={}
		local depres = builders_[v.restype](v.inputfiles, v.depfiles, localparameters, fullrespath)
        for _, drespath in pairs(depres) do
            local dpkg, dres = string.match(drespath, "(%u+)%.(%u+)")
            G.print(strfmt("Adding dependent resource %s to resource %s", drespath, v.respath))
            G.table.insert(v.dependentpackages, dpkg)
        end
        packagestouched[v.package]=1
	end
    
    for pkgname, _ in pairs(packagestouched) do
        G.print("Updating package info for "..pkgname)
        local packdir=G.buildpathresolve("dest://"..pkgname.."/DAT")
        local deppkgs={}
        local pkgxmlroot=xml.new()
        local resxmlnode=xml.new()
        local linkxmlnode=xml.new()
        pkgxmlroot:tag("package")
        resxmlnode:tag("resources")
        linkxmlnode:tag("packagelinks")
        for _, pkgres in pairs(packages_[pkgname].resources) do
            local resnode = xml.new({name=pkgres.name})
            resnode:tag("resource")
            resxmlnode:append(resnode)
            G.print(strfmt("Adding resource %s to package info", pkgres.name))
            if pkgres.dependentpackages ~= nil then
                for _, deppkg in pairs(pkgres.dependentpackages) do
                    deppkgs[deppkg]=1
                end
            end
        end
        for deppkg, _ in pairs(deppkgs) do
            local linknode=xml.new({name=deppkg})
            linknode:tag("link")
            linkxmlnode:append(linknode)
            G.print(strfmt("Adding package link %s to package info", deppkg))
        end
        pkgxmlroot:append(linkxmlnode)
        pkgxmlroot:append(resxmlnode)
        pkgxmlroot:save(packdir)
    end

    G.print("Build finished")
    G.print=oldprint
    G.buildpathresolve=nil
    logfile:flush()
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


