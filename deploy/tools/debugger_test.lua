local luadb = require "luadb"

function parsecmdline(args)
    script = ""
    port = 8335
    nextisport = false
    nextisscript = false
    for i,v in ipairs(arg) do 
        print(string.format("arg[%d]=%s", i, v))
        if nextisport then
            for w in string.gmatch(v, "%d+") do
                port = tonumber(w)
            end
            nextisport = false
        end
        if nextisscript then
            for w in string.gmatch(v, "%\"?(.+)%\"?") do
                nextisscript = false
                script = script..w
            end
        end
        if string.find(v, "-p") == 1 then
            nextisport = true
            for w in string.gmatch(v, "-p(%d+)") do
                nextisport = false
                port = tonumber(w)
            end
        end
        if string.find(v, "-d") == 1 then
            nextisscript = true
            for w in string.gmatch(v, "-d%\"?(.+)%\"?") do
                nextisscript = false
                script = w
            end
        end
    end
    
    return script, port
end

local script, port = parsecmdline(arg)

luadb.attach(port)

db=loadfile(script)
ok, errstr = xpcall(db, function (msg) print(msg) print(debug.traceback()) end)
luadb.dettach();