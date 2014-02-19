

local module = {}
local wx = require "wx"
ENV = nil

local wxT = function(s) return s end
local _ = function(s) return s end

local IDCounter = nil
local function newID()
    if not IDCounter then IDCounter = wx.wxID_HIGHEST end
    IDCounter = IDCounter + 1
    return IDCounter
end

return {
    newID = newID,
    text = wxT,
}