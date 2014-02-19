g_build_path = "add_all_package_resources.lua"

function buildallresources()
    local db=require "databuilder"
    db.buildresources(g_build_path)
end

function cleanbuilddata()
    os.remove("resources.d")
end

print (
[[Build system started...
Call "buildallresources()" to build data.
Call "cleanbuilddata()" to remove data time stamps
g_build_path variable contains the path of the build script. This is currently set to "]] ..g_build_path..[["]] )

buildallresources()
