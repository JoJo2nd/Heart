local filesystem = require "filesystem"
local process = require "lua_process"
local protobuf = require "proto_lua"

for k, v in ipairs(arg) do
    print(k, "=", v)
end

local G = _G
local buildables = {}
local data_path = filesystem.canonical("C:/dev/heart_lua/data/") --TODO: parse these from command line
local temp_data_path = data_path.."/.tmp"
local output_data_path = "C:/dev/heart_lua/deploy/gamedata"
local job_count = cores or 3              --TODO: parse these from command line
local verbose = false
local current_jobs = 0
local processes = {}
local success = 0
local failure = 0


if filesystem.isdirectory(temp_data_path) == false then
    os.execute("mkdir \""..temp_data_path.."\"")
end
if filesystem.isdirectory(output_data_path) == false then
    os.execute("mkdir \""..output_data_path.."\"")
end
temp_data_path = data_path.."/.tmp" --TODO: parse these from command line

local function verbose_log(str)
    if verbose then
        print("[LOG]:"..str)
    end
end

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

local function totablestring(t)
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
        print               = print,
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
    folder_path = filesystem.canonical(folder_path)
   
    local local_type_parameters = type_parameters or {}
    local local_package = package or ""
    local local_add_build_folder = function (local_folder_path)
        local ltp = deepcopy(local_type_parameters)
        add_build_folder(folder_path.."/"..local_folder_path, ltp, local_package)
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
            local full_wildcard = folder_path.."/"..wildcard
            local full_folder_path = filesystem.parentpath(full_wildcard)
            local dir_files = filesystem.readdir(full_folder_path)
            if dir_files == nil then return end
            local new_parameters = deepcopy(local_type_parameters[res_type]) or {}
            if type(type_param_or) == "table" then
                for k, v in pairs(type_param_or) do
                    new_parameters[k] = v
                end
            end
            for _, v in pairs(dir_files) do
                if filesystem.isfile(v) then
                    local res_path = filesystem.canonical(v)
                    if filesystem.wildcardpathmatch(full_wildcard, res_path) == true then
                        if buildables[res_path] == nil then 
                            buildables[res_path] = {}
                            buildables[res_path].full_path = res_path
                            buildables[res_path].res_id = string.gsub(filesystem.pathwithoutext(res_path), data_path, "")
                            buildables[res_path].package = local_package
                            buildables[res_path].res_type = res_type
                            buildables[res_path].parameters = new_parameters
                        else
                            verbose_log(string.format("Skipped resource %s because it's already added", res_path))
                        end
                    end
                end
            end
        end,
    }
    local build_script = folder_path.."/.build_script"
    if filesystem.isfile(build_script) then
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
            exit_code = pid:wait(0)
            if exit_code ~= nil then
                if exit_code ~= 0 then
                    failure = failure+1
                else
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
        packages[v.package].resources[string.gsub(k, data_path, "")] = v
        --add dependants = 
        local temp_file_name, output_file, dep_output_file = get_resource_filenames(v.package, k)
        dep_file = io.open(dep_output_file, "r")
        for line in dep_file:lines() do
            local path = filesystem.canonical(data_path..line)
            if buildable_resources[path] ~= nil then
                packages[v.package].depends[buildable_resources[path].package] = buildable_resources[path].package
                verbose_log(string.format("resource %s depends on %s", k, path))
            end
        end
        dep_file:close()
        verbose_log(string.format("adding resource %s to package %s", k, v.package))
    end
    
    -- Write out packages
    for k, v in pairs(packages) do
        local pkg_header = protobuf.Heart.proto.PackageHeader.new()
        for i, dep in pairs(v.depends) do
            -- TODO: fix this awful lua-protobuf-gen issue
            pkg_header:set_packagedependencies(pkg_header:size_packagedependencies()+1, dep)
        end
        local offset = 0
        for i, res in pairs(v.resources) do
            local temp_file_name, output_file, dep_output_file = get_resource_filenames(res.package, res.full_path)
            local entry = pkg_header:add_entries()
            local filesize = filesystem.filesize(output_file)
            entry:set_entryname(res.res_id)
            entry:set_entryoffset(offset)
            entry:set_entrysize(filesize)
            offset=offset+filesize
        end
        local codedoutputstream = protobuf.CodedOutputStream.new(output_data_path.."/"..k..".pkg")
        codedoutputstream:WriteVarint32(offset)
        codedoutputstream:WriteRaw(pkg_header:serialized())
        for i, res in pairs(v.resources) do
            local temp_file_name, output_file, dep_output_file = get_resource_filenames(res.package, res.full_path)
            local res = io.open(output_file, "rb")
            local data = res:read("*a")
            res:close()
            codedoutputstream:WriteRaw(data)
        end
    end
end

--- Exec

add_build_folder(data_path)

for k, v in pairs(buildables) do
    local temp_file_name, output_file, dep_output_file = get_resource_filenames(v.package, k)
    tmp_file = io.open(temp_file_name, "w")
    local paramstr = totablestring(v.parameters)
    local build_script = string.gsub([[
--print("Build - $inputfile")

local builder = require "$buildername"
local parameters =  $params
local inputfiles = builder.build("$inputfile", {}, parameters, "$tempoutputfile")
local depfile = io.open("$depoutputfile", "w")
for i, v in ipairs(inputfiles) do
    depfile:write(string.gsub(v, "$buildpath", "").."\n")
end
depfile:close()
    ]], "$(%w+)", {
        params = paramstr, 
        buildername=v.res_type, 
        inputfile=k, 
        tempoutputfile=output_file, 
        depoutputfile=dep_output_file,
        buildpath=data_path,
    })
    
    tmp_file:write(build_script)
    tmp_file:close()
    local exe = "lua"
    wait_for_free_job_slot(job_count)
    current_jobs = current_jobs + 1
    print(string.format("Building Resource - [%s]%s", v.package, string.gsub(k, data_path, "")))
    processes[k] = process.exec(string.format("%s \"%s\"", exe, temp_file_name))
end

--Wait for it all to be done
wait_for_free_job_slot(0)

print(string.format("Build complete: %d Successfully Completed, %d Failures", success, failure))

-- Output packages
write_packages(buildables)
