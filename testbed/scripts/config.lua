cvar("plugin.filesystem", "hddfilesystem")
cvar("plugin.renderer", "opengl_es2")

cvar("renderer.fullscreen", false)
cvar("renderer.vsync", true)
-- cvar("renderer.width", 1280) -- Old values not used by renderer anymore?
-- cvar("renderer.height", 720) -- Old values not used by renderer anymore?
cvar("window.sizex", 1280)
cvar("window.sizey", 720)

cvar("startup.script", "/script/startup.lua")

cvar("debug.server.port", 5682)

cvar("renderer.gl.multithreaded", 0)
cvar("renderer.scratchmemsize", 4*1024*1024) --4MB
--cvar("renderer.gl.notexturestorage", true) -- disables immutable texture storage. TODO: this doesn't work yet...
cvar("renderer.gl.nobufferstorage", true) -- disables persistant buffer mappings.
--cvar("renderer.gl.nosamplerobjects", true) -- disables sampler objects, forcing older style GL sampler setup. TODO: implementO
