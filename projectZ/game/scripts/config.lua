cvar("plugin.filesystem", "hddfilesystem")
cvar("plugin.renderer", "d3d9renderer")

cvar("renderer.fullscreen", false)
cvar("renderer.vsync", true)
-- cvar("renderer.width", 1280) -- Old values not used by renderer anymore?
-- cvar("renderer.height", 720) -- Old values not used by renderer anymore?
cvar("window.sizex", 1280)
cvar("window.sizey", 720)

cvar("startup.script", "/scripts/startup.lua")

cvar("filesystem.scripts_dir", "/projectz/scripts");
cvar("filesystem.data_dir", "/projectz/data");
cvar("filesystem.tmp_dir", "/projectz/tmp");
cvar("filesystem.save_dir", "/projectz/save");

cvar("debug.server.port", 5682)

cvar("renderer.gl.multithreaded", 1)
cvar("renderer.scratchmemsize", 4*1024*1024) --4MB
--cvar("renderer.gl.notexturestorage", true) -- disables immutable texture storage. TODO: this doesn't work yet...
--cvar("renderer.gl.nobufferstorage", true) -- disables persistant buffer mappings.
--cvar("renderer.gl.nosamplerobjects", true) -- disables sampler objects, forcing older style GL sampler setup. TODO: implementO
