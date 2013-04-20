local db=require "databuilder"

function printtablekeys(t)
    for k,v in pairs(t) do
        print(string.format("t[%s]=%s", tostring(k), tostring(v)))
    end
end

function buildallresources(path)
    if not path then path = "build_scripts/add_all_package_resources.lua" end
    db.buildresources({namematch=".+"}, path)
end