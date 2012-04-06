project "freetype"
	location (ProjectDir)
	kind "StaticLib"
	language "C"
	files {
		"../freetype-2.4.8/include/**/ft2build.h",
		"../freetype-2.4.8/include/**/ftconfig.h",
		"../freetype-2.4.8/include/**/fhheader.h",
		"../freetype-2.4.8/include/**/ftmodule.h",
		"../freetype-2.4.8/include/**/ftoption.h",
		"../freetype-2.4.8/include/**/ftstdlib.h",
		"../freetype-2.4.8/src/**/ftbbox.c",
		"../freetype-2.4.8/src/**/ftgcval.c",
		"../freetype-2.4.8/src/**/ftlcdfil.c",
		"../freetype-2.4.8/src/**/ftmm.c",
		"../freetype-2.4.8/src/**/ftotval.c",
		"../freetype-2.4.8/src/**/ftpatent.c",
		"../freetype-2.4.8/src/**/ftpfr.c",
		"../freetype-2.4.8/src/**/ftsynth.c",
		"../freetype-2.4.8/src/**/fttype1.c",
		"../freetype-2.4.8/src/**/ftwinfnt.c",
		"../freetype-2.4.8/src/**/ftxf86.c",
		"../freetype-2.4.8/src/**/pcf.c",
		"../freetype-2.4.8/src/**/pfr.c",
		"../freetype-2.4.8/src/**/psaux.c",
		"../freetype-2.4.8/src/**/pshinter.c",
		"../freetype-2.4.8/src/**/psmodule.c",
		"../freetype-2.4.8/src/**/raster.c",
		"../freetype-2.4.8/src/**/sfnt.c",
		"../freetype-2.4.8/src/**/truetype.c",
		"../freetype-2.4.8/src/**/type1.c",
		"../freetype-2.4.8/src/**/type1cid.c",
		"../freetype-2.4.8/src/**/type42.c",
		"../freetype-2.4.8/src/**/winfnt.c",
		"../freetype-2.4.8/src/**/autofit.c",
		"../freetype-2.4.8/src/**/bdf.c",
		"../freetype-2.4.8/src/**/cff.c",
		"../freetype-2.4.8/src/**/ftbase.c",
		"../freetype-2.4.8/src/**/ftbitmap.c",
		"../freetype-2.4.8/src/**/ftcache.c",
		"../freetype-2.4.8/src/**/ftdebug.c",
		"../freetype-2.4.8/src/**/ftfstype.c",
		"../freetype-2.4.8/src/**/ftgasp.c",
		"../freetype-2.4.8/src/**/ftglyph.c",
		"../freetype-2.4.8/src/**/ftgzip.c",
		"../freetype-2.4.8/src/**/ftinit.c",
		"../freetype-2.4.8/src/**/ftlzw.c",
		"../freetype-2.4.8/src/**/ftstroke.c",
		"../freetype-2.4.8/src/**/ftsystem.c",
		"../freetype-2.4.8/src/**/smooth.c"}
	defines {CommonDefines}
	defines {"FT2_BUILD_LIBRARY"}
	includedirs {"../freetype-2.4.8/include"}
	
configuration (DebugCfgName)
	targetdir (TargetDir..DebugCfgName)
	defines {{DebugDefines}}
	flags {"Symbols"}
configuration (ReleaseCfgName)
	targetdir (TargetDir..ReleaseCfgName)
	defines {{ReleaseDefines}}
	flags {"Optimize"}