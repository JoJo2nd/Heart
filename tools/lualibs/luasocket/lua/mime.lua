-----------------------------------------------------------------------------
-- MIME support for the Lua language.
-- Author: Diego Nehab
-- Conforming to RFCs 2045-2049
-- RCS ID: $Id: mime.lua,v 1.15 2004/06/21 06:07:57 diego Exp $
-----------------------------------------------------------------------------

-----------------------------------------------------------------------------
-- Load other required modules
-----------------------------------------------------------------------------
local mime = requirelib("mime", "luaopen_mime", getfenv(1))
local ltn12 = require("ltn12")

-- encode, decode and wrap algorithm tables
mime.encodet = {}
mime.decodet = {}
mime.wrapt = {}

-- creates a function that chooses a filter by name from a given table 
local function choose(table)
    return function(name, opt1, opt2)
        if type(name) ~= "string" then 
            name, opt1, opt2 = "default", name, opt1
        end
        local f = table[name or "nil"]
        if not f then error("unknown key (" .. tostring(name) .. ")", 3)
        else return f(opt1, opt2) end
    end
end

-- define the encoding filters
mime.encodet['base64'] = function()
    return ltn12.filter.cycle(b64, "")
end

mime.encodet['quoted-printable'] = function(mode)
    return ltn12.filter.cycle(qp, "", 
        (mode == "binary") and "=0D=0A" or "\r\n")
end

-- define the decoding filters
mime.decodet['base64'] = function()
    return ltn12.filter.cycle(unb64, "")
end

mime.decodet['quoted-printable'] = function()
    return ltn12.filter.cycle(unqp, "")
end

-- define the line-wrap filters
mime.wrapt['text'] = function(length)
    length = length or 76
    return ltn12.filter.cycle(wrp, length, length) 
end
mime.wrapt['base64'] = wrapt['text']
mime.wrapt['default'] = wrapt['text']

mime.wrapt['quoted-printable'] = function()
    return ltn12.filter.cycle(qpwrp, 76, 76) 
end

-- function that choose the encoding, decoding or wrap algorithm
mime.encode = choose(encodet) 
mime.decode = choose(decodet)
mime.wrap = choose(wrapt)

-- define the end-of-line normalization filter
function mime.normalize(marker)
    return ltn12.filter.cycle(eol, 0, marker)
end

-- high level stuffing filter
function mime.stuff()
    return ltn12.filter.cycle(dot, 2)
end
