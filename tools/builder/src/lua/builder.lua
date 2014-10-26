local filesystem = require "filesystem"
local process = require "lua_process"
local protobuf = require "proto_lua"

--for k, v in ipairs(arg) do
--    print(k, "=", v)
--end

local G = _G
local buildables = {}
local data_path = string.gsub(in_data_path, "\\", "/");
local output_data_path = string.gsub(in_output_data_path, "\\", "/");
local job_count = in_cores or 3                                  
local verbose = in_verbose                               
local current_jobs = 0
local processes = {}
local success = 0
local cached = 0
local failure = 0
local temp_data_path = string.gsub(data_path.."/.tmp", "\\", "/");
local cache_data_path = string.gsub(data_path.."/.cache", "\\", "/");

local function verbose_log(str, ...)
    if verbose == true then
        print(string.format(str, ...))
    end
end
local function error_log(str, ...)
    -- 31 for red
    red = "\x1B[31m"
    reset = "\x1B[0m"
    str = string.format(str, ...)
    print(red..str..reset)
end
local function log(str, ...)
    print(string.format(str, ...))
end


data_path = filesystem.canonical(data_path);
output_data_path = filesystem.canonical(output_data_path);
temp_data_path = filesystem.canonical(temp_data_path);
cache_data_path = filesystem.canonical(cache_data_path);

if filesystem.isdirectory(temp_data_path) == false then
    filesystem.makedirectories(temp_data_path)
end
if filesystem.isdirectory(output_data_path) == false then
    filesystem.makedirectories(output_data_path)
end
if filesystem.isdirectory(cache_data_path) == false then
    filesystem.makedirectories(cache_data_path)
end

log("Build data path %s", data_path)
log("Build output path %s", output_data_path)
log("Build tmp path %s", temp_data_path)
log("Build cache path %s", cache_data_path)
log("Build core count %s", job_count)

verbose_log("verbose log is on")

local function deepcopy(t)
    if type(t) ~= 'table' then return t end
    local mt = getmetatable(t)
    local res = {}
    for k,v in pairs(t) do
        if type(v) == 'table' then
            v = deepcopy(v)
        end
        res[k] = v
    end
    setmetatable(res,mt)
    return res
end

local function totablestring_old(t)
    local s = "{\n"
    for k, v in pairs(t) do
        if type(v) == "string" then
            s = s.."\t[\""..k.."\"]= \""..v.."\",\n"
        elseif type(v) == "table" then
            local ts = totablestring(v)
            s = s.."\t[\""..k.."\"]= "..ts..",\n"
        else
            s = s.."\t[\""..k.."\"]= "..tostring(v)..",\n"
        end
    end
    s = s.."}\n"
    return s
end

local function totablestring(t)
    local e = {}
    for k, v in pairs(t) do
        if type(v) == "string" then
            table.insert(e, "\t[\""..k.."\"]= \""..v.."\",\n");
        elseif type(v) == "table" then
            local ts = totablestring(v)
            table.insert(e,"\t[\""..k.."\"]= "..ts..",\n")
        else
            table.insert(e, "\t[\""..k.."\"]= "..tostring(v)..",\n")
        end
    end

    table.sort(e)
    local s = "{\n"
    for k, v in pairs(e) do
        s = s..v
    end
    s = s.."}\n"
    return s
end

local function add_build_folder(folder_path, type_parameters, package)
    local env = {
        filesystem          = filesystem,
        assert              = assert,
        collectgarbage      = collectgarbage,
        --dofile,            = --dofile,
        error               = error,
        getmetatable        = getmetatable,
        ipairs              = ipairs,
        --load               = --load
        --loadfile           = --loadfile
        next                = next,
        pairs               = pairs,
        --pcall              = --pcall
        print               = log,
        rawequal            = rawequal,
        rawget              = rawget,
        rawlen              = rawlen,
        rawset              = rawset,
        --require            = --require
        select              = select,
        setmetatable        = setmetatable,
        tonumber            = tonumber,
        tostring            = tostring,
        type                = type,
        --xpcall             = --xpcall
    }
    folder_path = folder_path
    verbose_log("adding build folder %s", folder_path)
    local local_type_parameters = type_parameters or {}
    local local_package = package or ""
    local local_add_build_folder = function (local_folder_path)
        local ltp = deepcopy(local_type_parameters)
        verbose_log("add_build_folder(%s) called", local_folder_path)
        add_build_folder(string.format("%s/%s",folder_path,local_folder_path), ltp, local_package)
    end
    env.buildsystem = {
        add_build_folder = local_add_build_folder,
        set_current_package = function (new_package)
            assert(type(new_package) == "string")
            local_package = new_package
        end,
        set_type_parameter = function(res_type, parameter, value)
            if local_type_parameters[res_type] == nil then local_type_parameters[res_type] = {} end
            local_type_parameters[res_type][parameter] = value
        end,
        add_files = function(wildcard, res_type, type_param_or)
            verbose_log("add_files(%s, %s, %s)", wildcard, res_type, type_param_or)
            local full_wildcard = string.format("%s/%s",folder_path, wildcard)
            local full_folder_path = filesystem.parentpath(full_wildcard)
            local dir_files = filesystem.readdir(full_folder_path)
            if dir_files == nil then
                verbose_log("filesystem.readdir(%s) returned nothing", full_folder_path)
                return
            end
            verbose_log("filesystem.readdir(%s) returned %d", full_folder_path, #dir_files)
            for _, v in pairs(dir_files) do
                verbose_log("readdir returned %s", v)
            end
            local new_parameters = deepcopy(local_type_parameters[res_type]) or {}
            if type(type_param_or) == "table" then
                for k, v in pairs(type_param_or) do
                    new_parameters[k] = v
                end
            end
            for _, v in pairs(dir_files) do
                verbose_log("Checking for file %s", v)
                if filesystem.isfile(v) then
                    verbose_log("File %s exists", v)
                    local res_path = v
                    local res_id = string.gsub(filesystem.pathwithoutext(res_path), data_path, "")
                    verbose_log("res_id = %s, res_path = %s, wildcard = %s", res_id, res_path, full_wildcard)
                    if filesystem.wildcardpathmatch(full_wildcard, res_path) == true then
                        verbose_log("File %s matches wildcard %s", v, full_wildcard)
                        if buildables[res_id] == nil then
                            buildables[res_id] = {}
                            buildables[res_id].full_path = res_path
                            buildables[res_id].res_id = res_id
                            buildables[res_id].package = local_package
                            buildables[res_id].res_type = res_type
                            buildables[res_id].parameters = new_parameters
                        else
                            verbose_log("Skipped resource %s because it's already added", res_path)
                        end
                    end
                end
            end
        end,
    }
    local build_script = string.format("%s/%s",folder_path,".build_script")
    verbose_log("checking for build script %s in %s", build_script, folder_path)
    if filesystem.isfile(build_script) then
        verbose_log("found build script %s", build_script)
        local build, errmsg = loadfile(build_script, "t", env)
        if build == nil then
            error("unable to run build script '"..build_script.."'. Error: "..errmsg)
        else
            build()
        end
    else
        error("folder "..build_script.." is missing a .build_script file")
    end
end

-- create file name that dependant resources are listed in
local function get_resource_filenames(package, filepath)
    local temp_file_name = string.format("%s_%s", package, filesystem.pathwithoutext(filepath))
    temp_file_name = string.gsub(temp_file_name, data_path, "")
    temp_file_name = string.gsub(temp_file_name, "[\\/:.]", "_")
    local output_file = string.format("%s/%s.bin", temp_data_path, temp_file_name)
    local dep_output_file = string.format("%s/%s.dep", temp_data_path, temp_file_name)
    temp_file_name = string.format("%s/%s.lua", temp_data_path, temp_file_name)
    
    return temp_file_name, output_file, dep_output_file
end

-- Wait for pending processes to be less or equal job_limit
local wait_for_free_job_slot = function(job_limit)
    while current_jobs > job_limit do
        for proc, pid in pairs(processes) do
            exit_code = pid.p:wait(0)
            if exit_code ~= nil then
                if exit_code ~= 0 then
                    local f=io.open(pid.l, "r")
                    local e = f:read('*a')
                    f:close()
                    error_log(e)
                    failure = failure+1
                else
                    os.remove(pid.l)
                    success = success+1
                end
                processes[proc] = nil
                current_jobs = current_jobs - 1
                break
            else
                process.sleep(1)
            end
        end
    end
end

function write_packages(buildable_resources)
    local packages = {}
    -- Build a list of reseources in packages 
    for k, v in pairs(buildable_resources) do
        if packages[v.package] == nil then packages[v.package] = {depends={}, resources={}} end
        packages[v.package].resources[v.res_id] = v
        --add dependants = 
        local temp_file_name, output_file, dep_output_file = get_resource_filenames(v.package, v.full_path)
        dep_file = io.open(dep_output_file, "r")
        for line in dep_file:lines() do
            local path = filesystem.pathwithoutext(line)
            if buildable_resources[line] ~= nil then
                packages[v.package].depends[buildable_resources[line].package] = buildable_resources[line].package
                verbose_log("resource %s requires %s to link", v.full_path, line)
            end
        end
        dep_file:close()
        verbose_log("adding resource %s to package %s header", v.full_path, v.package)
    end
    
    -- Write out packages
    for k, v in pairs(packages) do
        log("Writing package %s", k)
        local pkg_header = protobuf.Heart.proto.PackageHeader.new()
        for i, dep in pairs(v.depends) do
            if k ~= dep then
                verbose_log("Writing dependency %s", dep)
                -- TODO: fix this awful lua-protobuf-gen issue
                pkg_header:set_packagedependencies(pkg_header:size_packagedependencies(), dep)
            end
        end
        local offset = 0
        for i, res in pairs(v.resources) do
            local temp_file_name, output_file, dep_output_file = get_resource_filenames(res.package, res.full_path)
            local entry = pkg_header:add_entries()
            local filesize = filesystem.filesize(output_file)
            verbose_log("Writing resource entry %s. Temp name:%s, output file:%s, dep file:%s, filesize:%d", 
                res.full_path, temp_file_name, output_file, dep_output_file, filesize)
            entry:set_entryname(res.res_id)
            entry:set_entryoffset(offset)
            entry:set_entrysize(filesize)
            entry:set_entrytype(res.res_type)
            offset=offset+filesize
        end
        local codedoutputstream = protobuf.CodedOutputStream.new(string.format("%s/%s.pkg",output_data_path,k))
        verbose_log("Created coded output stream %s @ "..tostring(codedoutputstream), string.format("%s/%s.pkg",output_data_path,k))
        local header_str, header_size = pkg_header:serialized()
        codedoutputstream:WriteVarint32(header_size) -- or #header_str
        codedoutputstream:WriteRaw(header_str)
        verbose_log("Written header. Size:%d, Resources:%d", header_size, #v.resources)
        for i, res in pairs(v.resources) do
            local temp_file_name, output_file, dep_output_file = get_resource_filenames(res.package, res.full_path)
            verbose_log("Reading resource file %s", output_file)
            local res = io.open(output_file, "rb")
            local data = res:read("*a")
            res:close()
            verbose_log("Writing resource data %s of lenght %d", output_file, #data)
            codedoutputstream:WriteRaw(data)
        end
    end
end

--- Exec

local global_file_hash_lookup = {}
for k, v in pairs(filesystem.readdir('.')) do
    local filename = filesystem.filewithext(v)
    if filename then
        if string.find(filename, ".dll") or string.find(filename, ".so") or string.find(filename, ".lua") then
            global_file_hash_lookup[filesystem.pathwithoutext(filename)] = filesystem.fileMD5(filename);
        end
    end
end

for k, v in pairs(filesystem.readdirrecursive(data_path)) do
    if (filesystem.isfile(v)) then
        global_file_hash_lookup[v] = filesystem.fileMD5(v);
    end
end

for k, v in pairs(global_file_hash_lookup) do verbose_log("%s=%s", k, v) end

add_build_folder(data_path)

for k, v in pairs(buildables) do
    local temp_file_name, output_file, dep_output_file = get_resource_filenames(v.package, v.full_path)
    local error_log_filename = output_file..'.log'
    tmp_file = io.open(temp_file_name, "w")
    local paramstr = totablestring(v.parameters)
    local build_script = string.gsub([[
local errorlog = io.open("$errorlog", "w")
local function error_print (str,...)
    errorlog:write(string.format(str, ...))
end
print = error_print
print("Building - $inputfile\n")

local filesystem = require "filesystem"
local builder = require "$buildername"
local parameters =  $params
local inputfiles = builder.build("$inputfile", {}, parameters, "$tempoutputfile")
local depfile = io.open("$depoutputfile", "w")
-- Write out builder dll/so
depfile:write("$buildername".."?$hash".."\n")
-- Write out this lua file
p = filesystem.canonical("$buildscript")
if (filesystem.isfile(p)) then
    depfile:write(p.."?"..filesystem.fileMD5(p).."\n")
end
--Write out any deps
for i, v in ipairs(inputfiles) do
    p = filesystem.canonical(v)
    if (filesystem.isfile(p)) then
        depfile:write(p.."?"..filesystem.fileMD5(v).."\n")
    else
        depfile:write(v.."\n")
    end
end
depfile:close()
    ]], "$(%w+)", {
        params = paramstr, 
        buildscript=temp_file_name,
        buildername=v.res_type, 
        inputfile=v.full_path, 
        tempoutputfile=output_file, 
        errorlog=error_log_filename,
        depoutputfile=dep_output_file,
        buildpath=data_path,
        hash=global_file_hash_lookup[v.res_type]
    })
    
    tmp_file:write(build_script)
    tmp_file:flush()
    tmp_file:close()
    -- update the script hash now to detect any new/changed parameters
    global_file_hash_lookup[temp_file_name]=filesystem.fileMD5(temp_file_name)
    local dobuild = function () 
        local exe = "lua"
        wait_for_free_job_slot(job_count)
        current_jobs = current_jobs + 1
        log("Building Resource - [%s]%s", v.package, string.gsub(k, data_path, ""))
        processes[k] = {p=process.exec(string.format("%s \"%s\"", exe, temp_file_name)), l=error_log_filename, d=depoutputfile}
    end
    -- check the cache
    needsbuild = true
    dep_file = io.open(dep_output_file, "r")
    if dep_file then
        needsbuild = false
        for line in dep_file:lines() do
            local path, hash = string.match(line, '(.*)?(.*)')
            if global_file_hash_lookup[path] ~= hash then
                -- something is out of date, build
                verbose_log("%s=%s doesn't match %s on line %s", path, global_file_hash_lookup[path], hash, line)
                needsbuild = true
            end
        end
        
    end
    if needsbuild == true then
        dobuild()
    else
        cached = cached+1
        log("Using Cached Resource - [%s]%s", v.package, string.gsub(k, data_path, ""))
    end

end

--Wait for it all to be done
wait_for_free_job_slot(0)

log("Build complete: %d Successfully Completed, %d Cached, %d Failures", success, cached, failure)

-- Output packages
write_packages(buildables)

log("Finished Writing Packages.")
