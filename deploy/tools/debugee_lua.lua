math.randomseed(os.time())

local xml = require "LuaXml"
local luadb = require "luadb"
local dbclient = luadb.newclient()
local port = math.random(1000, 9999)

os.execute("start Release/lua.exe debugger_test.lua -d \"sort.lua\" -p"..port)
--if not dbclient:connect("127.0.0.1", 1234) then

while not dbclient:isconnected() do
    print("Connecting to localhost:"..port.."...")
    dbclient:connect("127.0.0.1", port)
end

print (
[[Debugger commands...
r - Run
s - step
si - step in to current function
so - step out of current function
bt - print stack backtrace
]])

while dbclient:isconnected() and not dbclient:debuggerexit() do
    dbclient:tick(1000)
--[
    local cmd=luadb.readstdinasync()
    if cmd ~= nil then
        if cmd == "r" then
            dbclient:run()
        elseif cmd == "s" then
            dbclient:step()
        elseif cmd == "si" then
            dbclient:stepin()
        elseif cmd == "so" then
            dbclient:stepout()
        elseif cmd == "bt" then
            for i, v in ipairs(dbclient:backtrace()) do
                print(string.format("%d:%s[%s]: %s(%d)", i, v.name, v.what, v.shortsrc, v.currentline))
            end
        end
    end
--]]
end

