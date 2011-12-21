/********************************************************************

	filename: 	LuaScriptBuilder.cpp	
	
	Copyright (c) 8:5:2011 James Moran
	
	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.
	
	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
	
	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.
	
	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.
	
	3. This notice may not be removed or altered from any source
	distribution.

*********************************************************************/

#include "LuaScriptBuilder.h"
#include "ResourceFileSystem.h"
extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
struct LReader
{
	ResourceFile*	file_;
	char*			buf_;
	bool			done_;	
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const char* LuaReader( lua_State *L, void *ud, size_t *sz )
{
	LReader* r = (LReader*)ud;
	if ( r->done_ )
	{
		delete r->buf_;
		*sz = 0; 
		return NULL;
	}

	*sz = r->file_->Size();
	r->buf_ = new char[*sz];

	*sz = r->file_->Read( r->buf_, *sz );

	return r->buf_;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
int LuaScriptBuilder::LuaWriter( lua_State *L, const void* p, size_t sz, void* ud )
{
	LuaScriptBuilder* rb = (LuaScriptBuilder*)ud;
	rb->addDataToBuffer( p, sz );

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// 10:32:58 ////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool LuaScriptBuilder::BuildResource()
{
	ResourceFile* pInFile = pResourceFileSystem()->OpenFile( inputFilename_.c_str(), FILEMODE_READ );//

	if ( !pInFile )
	{
		ThrowFatalError( "Couldn't open Input %s file for " __FUNCTION__, inputFilename_.c_str() );
	}

	lua_State* lua = luaL_newstate();
	luaL_openlibs( lua );

	LReader reader;
	reader.file_ = pInFile;
	reader.done_ = false;
	reader.buf_ = NULL;

	int ret = lua_load( lua, &LuaReader, &reader, inputFilename_.c_str() );

	if ( ret != 0 )
	{
		ThrowFatalError( "Lua Compiling Error: %s", lua_tostring( lua, -1 ) );
	}

	// dump the compiled block thats on top of the lua stack
	//lua_dump( lua, &LuaScriptBuilder::LuaWriter, this );
	// dump the uncompiled text block, 
	pInFile->Seek( 0, ResourceFile::SO_SEEK_SET );
	hUint32 sz = pInFile->Size();
	char* buf = new char[sz];

	sz = pInFile->Read( buf, sz );

	addDataToBuffer( buf, sz );

	delete buf;

	lua_close( lua );

	return hTrue;
}
