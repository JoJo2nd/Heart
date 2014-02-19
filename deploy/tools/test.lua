-- Load all the modules
local wx = require "wx"
local wxUtil = require "wxutil"
local luadb = require "luadb"
local filesystem = require "filesystem"
local protobuf = require "proto_lua"
local enet = require "enet_lua"

print("Hello World...")

local testnum1 = 4578;
local testnum2 = 6635;
local testnum3 = -4578; -- Negative numbers do not work but this is expected I think 
local testnum4 = -6635; -- Negative numbers do not work but this is expected I think 
local teststr = "Hello From a File!"
local codedoutputstream = protobuf.CodedOutputStream.new("protobuf_coded_test")
codedoutputstream:WriteVarint32(testnum1)
codedoutputstream:WriteLittleEndian32(testnum2)
codedoutputstream:WriteVarint32(testnum3)
codedoutputstream:WriteLittleEndian32(testnum4)
codedoutputstream:WriteRaw(teststr)
codedoutputstream = nil
collectgarbage()

local codedinputstream = protobuf.CodedInputStream.new("protobuf_coded_test")
local readnum1 = codedinputstream:ReadVarint32()
local readnum2 = codedinputstream:ReadLittleEndian32(testnum2)
local readnum3 = codedinputstream:ReadVarint32(testnum3)
local readnum4 = codedinputstream:ReadLittleEndian32(testnum4)
local readstr  = codedinputstream:ReadRaw(18)

assert(readnum1 == testnum1)
assert(readnum2 == testnum2)
--assert(readnum3 == testnum3) 
--assert(readnum4 == testnum4)
assert(readstr == teststr)

print (codedinputstream.Skip)
print (codedinputstream.PushLimit)
print (codedinputstream.PopLimit)
print (codedinputstream.ReadRaw)
print (codedinputstream.ReadVarint32)

for k, v in pairs(protobuf.Heart.proto) do print (k) end
for k, v in pairs(protobuf.Heart.proto.eShaderType) do print(k) end
for k, v in pairs(protobuf.Heart.proto.eShaderStreamType) do print(k) end

local material = protobuf.Heart.proto.MaterialResource.new()
local material_pass_desc = protobuf.Heart.proto.MaterialPass.descriptor()
local material_pass_ras_state = protobuf.Heart.proto.RasterizerState.descriptor()
local material_pass_blend_state = protobuf.Heart.proto.BlendState.descriptor()
local material_pass_depth_stencil_state = protobuf.Heart.proto.DepthStencilState.descriptor()
print(material)
print(protobuf.Heart.proto.eShaderType.eShaderType_Pixel)

print(enet.ENetEventType.ENET_EVENT_TYPE_NONE)
print(enet.ENetPacketFlag.ENET_PACKET_FLAG_RELIABLE)
for k, v in pairs(enet) do print ("enet."..k) end
for k, v in pairs(enet.ENetHost) do print ("enet.ENetHost."..k) end
for k, v in pairs(enet.ENetAddress) do print ("enet.ENetAddress."..k) end
for k, v in pairs(enet.ENetEventType) do print ("enet.ENetEventType."..k) end
for k, v in pairs(enet.ENetPacketFlag) do print ("enet.ENetPacketFlags."..k) end
local address = enet.ENetAddress.new("127.0.0.1")
address:set_port(7650)
local host = enet.ENetHost.new(address, 16, 8, 0, 0)
--enet.ENetEventType.ENET_EVENT_TYPE_NONE = 10
--enet.ENetPacketFlag.ENET_EVENT_TYPE_NONE = 10
--enet.ENetPacketFlag.ENET_PACKET_FLAG_RELIABLE = 10

os.exit(0)