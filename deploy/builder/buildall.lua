
print("Running builder....")
local db=require "databuilder"
if not path then path = "build_scripts/add_all_package_resources.lua" end
db.buildresources(path)

function buildallresources(path)
    if not path then path = "build_scripts/add_all_package_resources.lua" end
    db.buildresources(path)
end