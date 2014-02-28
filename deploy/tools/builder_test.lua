local filesystem = require "filesystem"
local process = require "lua_process"

local G = _G
local buildables = {}
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
    if folder_path[#folder_path] ~= '/' then
        folder_path = folder_path..'/'
    end
   
    local local_type_parameters = type_parameters or {}
    local local_package = package or ""
    local local_add_build_folder = function (local_folder_path)
        local ltp = deepcopy(local_type_parameters)
        add_build_folder(folder_path..local_folder_path, ltp, local_package)
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
            local full_wildcard = folder_path..wildcard
            local full_folder_path = filesystem.parentpath(full_wildcard)
            local dir_files = filesystem.readdir(full_folder_path)
            local new_parameters = deepcopy(local_type_parameters[res_type]) or {}
            if type(type_param_or) == "table" then
                for k, v in pairs(type_param_or) do
                    new_parameters[k] = v
                end
            end
            for _, v in pairs(dir_files) do
                if filesystem.isfile(v) then
                    local res_path = v
                    if filesystem.wildcardpathmatch(full_wildcard, res_path) == true then
                        if buildables[res_path] == nil then buildables[res_path] = {} end
                        buildables[res_path].package = local_package
                        buildables[res_path].res_type = res_type
                        buildables[res_path].parameters = new_parameters
                    end
                end
            end
        end,
    }
    local build_script = folder_path..".build_script"
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

for k, v in ipairs(arg) do
    print(k, "=", v)
end

local data_path = "C:/dev/heart_lua/data" --TODO: parse these from command line
local temp_data_path = data_path.."/.tmp" --TODO: parse these from command line
local job_count = cores or 8              --TODO: parse these from command line


local current_jobs = 0
if filesystem.isdirectory(temp_data_path) == false then
    os.execute("mkdir \""..temp_data_path.."\"")
end

add_build_folder(data_path)

local processes = {}
local success = 0
local failure = 0

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

for k, v in pairs(buildables) do
    local temp_file_name = string.format("%s_%s", v.package, k)
    temp_file_name = string.gsub(temp_file_name, data_path, "")
    temp_file_name = string.gsub(temp_file_name, "[\\/:.]", "_")
    local output_file = string.format("%s/%s.bin", temp_data_path, temp_file_name)
    temp_file_name = string.format("%s/%s.lua", temp_data_path, temp_file_name)
    tmp_file = io.open(temp_file_name, "w")
    local paramstr = "{\n"
    for pk, pv in pairs(v.parameters) do
        if type(pv) == "string" then
            paramstr = paramstr.."\t[\""..pk.."\"]= \""..pv.."\",\n"
        else
            paramstr = paramstr.."\t[\""..pk.."\"]= "..tostring(pv)..",\n"
        end
    end
    paramstr = paramstr.."}\n"
    local build_script = string.gsub([[
--print("Build - ${inputfile}")

--local builder = require "${buildername}"
--local parameters =  ${params_x}
--builder.build("${inputfile}", {}, parameters, "${tempoutputfile}")
    ]], "${(%w+)}", {params = paramstr, buildername=v.res_type, inputfile=k, tempoutputfile=output_file})
    
    tmp_file:write(build_script)
    tmp_file:close()
    local exe = "lua"
    wait_for_free_job_slot(job_count)
    current_jobs = current_jobs + 1
    processes[k] = process.exec(string.format("%s \"%s\"", exe, temp_file_name))
end

--Wait for it all to be done
wait_for_free_job_slot(0)

process.sleep(500)

print(string.format("Build complete: %d Successfully Completed, %d Failures", success, failure))
