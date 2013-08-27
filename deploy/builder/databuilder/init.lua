local M = {}
local G = _G
-- pull in local modules
local filesystem = require "filesystem"
local xml = require "LuaXml"
local string = string
local table = table
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
local currentfilemodtime = filesystem.currentfiledate
local isfile = filesystem.isfile
-- disable access to globals
_ENV=nil

--local internal vars
local resources_={}
-- table of input files used by a resource entry e.g. resourceinputsdependencies_["PKG.RES"] = 
--~ table = {
--~    inputs = {file1.txt, file2.txt},
--~    links = {} table of resources that depend on this resource
--~    params = {} table of key, value pairs match the last parameters the resource was built with
--~    lastbuildtime = filemod
--~    }
local resourceinputsdependencies_={}
local builders_={}
local globaltypeoptions_={}
local packages_={}
local sourceRootDir_=""
local destRootDir_=""
local logfile=nil
local logerr=nil

logfile = io.open(strfmt("build_log_%s.txt",os.date("%Y-%m-%d")), "w")
logerr = io.open("errors.txt", "w")

local function buildPathResolve(x) 
    return strsub(x, "(%w+)://", {src=sourceRootDir_, dest=destRootDir_})
end
local function compareTables(at, bt)
    if #at ~= #bt then
        G.print("table compare fail, length mis-match")
        return false
    end
    for k,v in pairs(at) do
        if bt[k] == nil then
            G.print("table compare fail, b table missing key")
            return false
        end
        if type(at[k]) == "table" then
            if type(bt[k]) ~= "table" then
                return false
            end
            if not compareTables(at[k], bt[k]) then
                G.print("table compare fail, inner compare fail")
                return false
            end
        elseif tostring(bt[k]) ~= tostring(at[k]) then
            G.print("table compare fail, values don't match "..tostring(bt[k]).."~="..tostring(at[k]))
            return false
        end
    end
    return true
end

local function createResourceParameterTable(type, resparameters)
    local localparameters={}
    for paramname, param in pairs(globaltypeoptions_[type]) do
        localparameters[paramname] = param
    end
    for paramname, param in pairs(resparameters) do
        localparameters[paramname] = param
    end
    return localparameters
end

local function newResourceBuildInfo(time, parameters)
    if time == nil then
        time = currentfilemodtime()
    end
    if type(time) ~= "number" then
        time = tonumber(time)
    end
    if parameters == nil then
        parameters = {}
    end
    return {inputs={}, links={}, params=parameters, lastbuildtime=time}
end
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

local function generateResourcesForBuild()
    local resources={}
    local function buildNeeded(r, info)
        if info == nil then return true end
        if #r.inputfiles == 0 then return true end
        for _, file in pairs(r.inputfiles) do
            local realpath=buildPathResolve(file)
            if not isfile(realpath) then
                G.print(strfmt("File \"%s\"(real path: %s) doesn't exist. Resource \"%s\" will be built", file, realpath, r.respath))
                return true
            end
            if filemodtime(realpath) > info.lastbuildtime then
                G.print(strfmt("File \"%s\"(real path: %s) is new than resource \"%s\". Resource will be built", file, realpath, r.respath))
                return true
            end
        end
        local localparameters=createResourceParameterTable(r.restype, r.parameters)
        if not compareTables(info.params, localparameters) then
            G.print(strfmt("Resource \"%s\" parameter table doesn't match that in dependency file. Resource will be built", r.respath))
            return true
        end
        return false
    end
    local function gatherDependantResources(respath)
        local lastbuildinfo=resourceinputsdependencies_[respath]
        if lastbuildinfo ~= nil then
            for _, link in pairs(lastbuildinfo.links) do
                resources[link]=resources_[link]
                G.print(strfmt("Adding resource %s from link with %s", link, res.respath))
                gatherDependantResources(link)
            end
        end
    end
    for _, res in pairs(resources_) do
        local lastbuildinfo=resourceinputsdependencies_[res.respath]
        if buildNeeded(res, lastbuildinfo) then
            resources[res.respath]=res;
            gatherDependantResources(res.respath)
            G.print(strfmt("Adding resource %s for building", res.respath))
        else
            G.print(strfmt("Resource \"%s\" skipped.", res.respath))
        end
    end
    G.print("Finished looking for resources to build.")
    return resources
end

local function loadResourceInputDependencies()
    resourceinputsdependencies_={}
    if not isfile("resources.d") then
        return 
    end
    local depxml = xml.load("resources.d")
    local resources = depxml:find("resources")
    if resources ~= nil then
        for k,v in pairs(resources) do
            if v.tag ~= nil and v:tag() == "resource" then
                -- ensure there is an entry
                resourceinputsdependencies_[v.path]=newResourceBuildInfo(v.buildtime)
                for _, depvalue in pairs(v) do
                    if depvalue.tag ~= nil and depvalue:tag() == "dependency" then
                        table.insert(resourceinputsdependencies_[v.path].inputs, depvalue.path)
                    elseif depvalue.tag ~= nil and depvalue:tag() == "link" then
                        table.insert(resourceinputsdependencies_[v.path].links, depvalue.path)
                    elseif depvalue.tag ~= nil and depvalue:tag() == "parameter" then
                        resourceinputsdependencies_[v.path].params[depvalue.key]=depvalue.value
                    end
                end
            end
        end
    end
end

local function saveResourcesInputDependencies()
    local depxmlroot=xml.new()
    depxmlroot:tag("resources")
    for resourcepath, depinfo in pairs(resourceinputsdependencies_) do
        local resnode = xml.new()
        resnode:tag("resource")
        resnode.path=resourcepath
        resnode.buildtime=depinfo.lastbuildtime
        depxmlroot:append(resnode)
        for _, inputdep in pairs(depinfo.inputs) do
            local depnode = xml.new()
            depnode:tag("dependency")
            depnode.path=inputdep
            resnode:append(depnode)
        end
        for _, link in pairs(depinfo.links) do
            local linknode=xml.new()
            linknode:tag("link")
            linknode.path=link
            resnode:append(linknode)
        end
        for key, value in pairs(depinfo.params) do
            local paramnode=xml.new()
            paramnode:tag("parameter")
            paramnode.key=key
            paramnode.value=value
            resnode:append(paramnode)
        end
    end
    depxmlroot:save("resources.d")
end

local function resetResourceInputDependencies()
    resourceinputsdependencies_={}
end


local function resetDataBuild()
    resources_ = {}
    resourceinputsdependencies_={}
    builders_ = {}
    packages_ = {}
    globaltypeoptions_ = {}
    sourceRootDir_=nil
    destRootDir_=nil
end

local function buildResources(scriptpath)
    --create build log
    local oldprint=G.print
    G.print = function (...) 
        for i, v in ipairs{...} do
            logfile:write(G.tostring(v))
        end
        logfile:write("\n")
        oldprint(...)
    end 
    G.errorprint = function (...) 
        for i, v in ipairs{...} do
            logerr:write(G.tostring(v))
        end
        logerr:write("\n")
        G.print(...)
    end 
    G.splitresourceid = function (x)
        local dpkg, dres = string.match(drespath, "(%u+)%.(%u+)")
        return dpkg, dres
    end
    G.buildpathresolve = function (x) 
        return strsub(x, "(%w+)://", {src=sourceRootDir_, dest=destRootDir_})
    end
    G.resourcepathtofilepath = function (x)
        return strfmt("%s%s", destRootDir_, strsub(x, "%.", "/"))
    end
    G.abspathtosourcefilepath = function (x)
        return strsub(x, sourceRootDir_, "src://")
    end
    G.getresourceinputpath = function (x)
        local res = resources_[x]
        if (res == nil) then return nil end
        local retfiles = {}
        for _, v in pairs(res.inputfiles) do
            table.insert(retfiles, G.buildpathresolve(v))
        end
        return retfiles
    end
    local buildscript=nil
    local errmsg=nil
    G.print("//////////////////////////////////////////////////////////////////")
    G.print("//////////////////////////////////////////////////////////////////")
    G.print("//////////////////////////////////////////////////////////////////")
    G.print("//////////////////////////////////////////////////////////////////")
    G.print("//////////////////////////////////////////////////////////////////")
    G.print("//////////////////////////////////////////////////////////////////")
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

    loadResourceInputDependencies()
    local resourcestobuild=generateResourcesForBuild()
    local packagestouched={}
    local buildcoroutines={}
    local i=0
    G.errorprint("Error Log: Build Start... ")

    for k, v in pairs(resourcestobuild) do
        local co = coroutine.create( function ()
            local packdir="dest://"..v.package.."/"
            local fullrespath=packdir..v.name
            local realpath=G.buildpathresolve(packdir)
            local localparameters=createResourceParameterTable(v.restype, v.parameters)
            G.print(strfmt("Creating directory %s -> %s", packdir, realpath))
            filesystem.makedirectories(realpath)
            G.print(strfmt("Building \"%s\" resource %s to %s ", v.restype, v.respath, fullrespath))
            -- ensure there is an entry
            v.dependentpackages={}
            resourceinputsdependencies_[v.respath]=newResourceBuildInfo(nil, localparameters)
            local depres, depfiles, flushresifbuilt = builders_[v.restype](v.inputfiles, v.depfiles, localparameters, fullrespath)
            for _, drespath in pairs(depres) do -- list of resources this resource depends on. Causes the runtime to link them and ensure they load for this resource to use
                local dpkg, dres = string.match(drespath, "(%u+)%.(%u+)")
                G.table.insert(v.dependentpackages, dpkg)
            end
            if depfiles then -- list of files that the resource used to build, if these change the resource will be re-built
                for _, depfile in pairs(depfiles) do
                    local depfilepath=G.abspathtosourcefilepath(depfile);
                    G.table.insert(resourceinputsdependencies_[v.respath].inputs, depfilepath)
                end
            end
            if flushresifbuilt then -- list of resources that if they are built, this resource should be re-built too
                for _, drespath in pairs(flushresifbuilt) do
                    if resourceinputsdependencies_[drespath] == nil then
                        resourceinputsdependencies_[drespath]=newResourceBuildInfo()
                    end
                    G.table.insert(resourceinputsdependencies_[drespath].links, v.respath)
                end
            end
            packagestouched[v.package]=1
        end)
        buildcoroutines[i]=co
        i = i+1
    end
    local therewaserror=false
    local worktodo=true
    while worktodo do
        worktodo=false
        for k, co in pairs(buildcoroutines) do
            if (co~=nil) then
                worktodo=true
                local stat = coroutine.status(co)
                if stat == "suspended" then
                    local resumeok, er = coroutine.resume(co)
                    if not resoumeok and er ~= nil then 
                        therewaserror=true
                        G.errorprint("(>^o^)> BUILD ERROR <(^o^<) ",er)
                    end
                elseif stat == "dead" then
                    buildcoroutines[k] = nil
                end
            end
        end
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

    saveResourcesInputDependencies()
    
    G.errorprint("Error Log: Build finished")
    G.print("Build finished")
    G.print=oldprint
    G.buildpathresolve=nil
    logfile:flush()
    logerr:flush()
    
    if therewaserror then
        G.errorprint("Build completed with Errors!")
        os.execute("notepad errors.txt")
    end
end

_ENV=G

return { -- Module functions
    reset           =resetDataBuild,
    sourcerootpath  =getSourceRootPath,
    destrootpath    =getDestRootPath,
    buildresources  =buildResources,
    addresource     =addResource,
    addbuilder      =addBuilder,
}


