/********************************************************************

	filename: 	hRenderTechniqueManager.cpp	
	
	Copyright (c) 11:1:2012 James Moran
	
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

namespace Heart
{
    int keyCompare(const void* lhs, const void* rhs)
    {
        return *((hMaterialKeyContainer*)lhs) < *((hMaterialKeyContainer*)rhs) ? -1 : 1;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderMaterialManager::hRenderMaterialManager() 
        : matKeys_(NULL)
    {
        maxKeys_ = 64;
        nMatKeys_ = 0;
        matKeys_ = (hMaterialKeyContainer*)hHeapRealloc(GetGlobalHeap(), matKeys_, sizeof(hMaterialKeyContainer)*maxKeys_);
        constBlockLookUp_.SetAutoDelete(false);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderMaterialManager::~hRenderMaterialManager()
    {
        hHeapFreeSafe(GetGlobalHeap(), matKeys_);
        nMatKeys_ = maxKeys_ = 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hRenderTechniqueInfo* hRenderMaterialManager::AddRenderTechnique( const hChar* name )
    {
        hMutexAutoScope autoMtx( &accessMutex_ );

        hUint32 count = techniques_.GetSize();
        for ( hUint32 i = 0; i < count; ++i )
        {
            if ( hStrCmp( name, techniques_[i].name_ ) == 0 )
            {
                return &techniques_[i];
            }
        }

        hcAssert( techniques_.GetSize() < 30 );
        hUint32 maskC = techniques_.GetSize();
        hRenderTechniqueInfo newInfo;
        newInfo.mask_ = 1 << maskC;
        hStrCopy( newInfo.name_, newInfo.name_.GetMaxSize(), name );

        techniques_.PushBack( newInfo );
        return &techniques_[techniques_.GetSize()-1];
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hRenderTechniqueInfo* hRenderMaterialManager::GetRenderTechniqueInfo( const hChar* name )
    {
        hMutexAutoScope autoMtx( &accessMutex_ );

        hUint32 count = techniques_.GetSize();
        for ( hUint32 i = 0; i < count; ++i )
        {
            if ( hStrCmp( name, techniques_[i].name_ ) == 0 )
            {
                return &techniques_[i];
            }
        }

        return AddRenderTechnique(name);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdParameterConstantBlock* hRenderMaterialManager::GetGlobalConstantBlock(hUint32 id)
    {
        hGlobalConstantBlock* gcb = constBlockLookUp_.Find(id);
        return gcb ? gcb->constBlock_ : NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::GetUniqueKey( hMaterial* mat )
    {
        hMutexAutoScope autoMtx( &accessMutex_ );

        if (nMatKeys_+1 >= maxKeys_)
        {
            maxKeys_ *= 2;
            matKeys_ = (hMaterialKeyContainer*)hHeapRealloc(GetGlobalHeap(), matKeys_, sizeof(hMaterialKeyContainer)*maxKeys_);
        }

        //Walk the keys looking for a gap inbetween keys
        hUint32 key = 0;
        for (hUint32 i = 0; i < nMatKeys_; ++i)
        {
            if (matKeys_[i].key_ > key+1)
                break;
        }
        key += 1;
        hcAssertMsg((key&0x3FFFF)==key, "Run out of material keys, the max number of unique materials is %u", 0x3FFFF);
        matKeys_[nMatKeys_].mat_ = mat;
        matKeys_[nMatKeys_].key_ = (key&0x3FFFF);

        ++nMatKeys_;
        mat->uniqueKey_ = key;

        qsort(matKeys_, nMatKeys_, sizeof(hMaterialKeyContainer), keyCompare);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::RemoveKey( hMaterial* mat )
    {
        hMutexAutoScope autoMtx( &accessMutex_ );

        //Walk the keys looking for our key
        hUint32 key = mat->GetMatKey();
        for (hUint32 i = 0; i < nMatKeys_; ++i)
        {
            if (matKeys_[i].key_ == key)
            {
                --nMatKeys_;
                matKeys_[i] = matKeys_[nMatKeys_];
#ifdef HEART_DEBUG
                matKeys_[nMatKeys_].key_ = 0;
                matKeys_[nMatKeys_].mat_ = NULL;
#endif // HEART_DEBUG
                break;
            }
        }

        --nMatKeys_;

        qsort(matKeys_, nMatKeys_, sizeof(hMaterialKeyContainer), keyCompare);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::OpenLuaMaterialLib(lua_State* L)
    {
        static const luaL_Reg matlib[] = {
            {"registerRenderTechnique", RegisterRenderTechnique},
            {"registerParameterBlock", RegisterParameterBlock},
            {NULL, NULL}
        };

        lua_getglobal(L, "heart");
        lua_pushlightuserdata(L, this);
        luaL_setfuncs(L,matlib,1);
        lua_pop(L, 1);// pop heart module table
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    int hRenderMaterialManager::RegisterParameterBlock(lua_State* L)
    {
        hRenderMaterialManager* m = (hRenderMaterialManager*)lua_topointer(L, lua_upvalueindex(1));
        if (!m) luaL_error(L, "Unable to grab engine pointer" );

        luaL_checktype(L, -1, LUA_TTABLE);

        hGlobalConstantBlock block;

        lua_getfield(L, -1, "name"); // push name to stack top
        if (lua_type(L, -1) != LUA_TSTRING) luaL_error(L, "block.name must be a string type");
        block.name_ = lua_tostring(L, -1);
        block.nameHash_ = hCRC32::StringCRC(block.name_);
        block.strPoolSize_ = lua_rawlen(L, -1)+1;// get string len
        lua_pop(L, 1); // remove block.name
        
        lua_getfield(L, -1, "aliases");// push alias table to stack top
        if (lua_type(L, -1) != LUA_TTABLE) luaL_error(L, "block.aliases must be a table type");
        // create space for alias names, we get the pointers to these and copy them later
        block.aliasCount_ = lua_rawlen(L,-1);
        block.aliases_ = (const hChar**)hHeapMalloc(GetGlobalHeap(), sizeof(hChar*)*block.aliasCount_);
        for (hUint32 i = 0; i < block.aliasCount_; ++i)
        {
            lua_rawgeti(L, -1, (i+1));// push table entry onto stack (entries start at 1)
            if (lua_type(L, -1) != LUA_TSTRING) luaL_error(L, "alias must be a string");
            block.aliases_[i] = lua_tostring(L, -1);
            block.strPoolSize_ += lua_rawlen(L, -1)+1;// get string len
            lua_pop(L, 1);
        }
        lua_pop(L, 1);// Pop aliases table

        lua_getfield(L, -1, "parameters");// get parameters table
        if (lua_type(L, -1) != LUA_TTABLE) luaL_error(L, "expected parameter table");
        block.paramCount_ = lua_rawlen(L, -1);// get parameter count
        block.params_ = (hConstBlockParam*)hHeapMalloc(GetGlobalHeap(), sizeof(hConstBlockParam)*block.paramCount_);
        
        block.dataSize_ = 0;
        lua_pushnil(L);  // first key for enumeration
        for (hUint32 i = 0; lua_next(L, -2) != 0; ++i) 
        {
            // uses 'key' (key can be string or number/index) (at index -2) and 'value' (at index -1) 
            if (lua_type(L, -1) == LUA_TTABLE)
            {
                lua_getfield(L, -1, "name");// Get block.parameter.name
                if (lua_type(L, -1) != LUA_TSTRING) luaL_error(L, "parameter name field not found");
                block.params_[i].name_ = lua_tostring(L, -1);
                block.strPoolSize_ += lua_rawlen(L, -1)+1;// get string len
                block.params_[i].nameHash_ = hCRC32::StringCRC(block.params_[i].name_);

                lua_getfield(L, -2, "size");// Get block.parameter.size
                if (lua_isnumber(L, -1) == 0)  luaL_error(L,"parameter %s.size member is missing", block.params_[i].name_);
                block.params_[i].size_ = lua_tonumber(L, -1);
                block.params_[i].offset_ = block.dataSize_;
                block.dataSize_ += block.params_[i].size_;
                //TODO: initial data read
                lua_pop(L, 2);// Pop size and name
            }
            lua_pop(L, 1);// removes 'value'; keeps 'key' for next iteration
        }
        lua_pop(L, 2);// pop last key from enumeration and parameter table 
        
        //build the string pools
        block.strPool_ = (hChar*)hHeapMalloc(GetGlobalHeap(), block.strPoolSize_);
        block.data_ = hHeapMalloc(GetGlobalHeap(), block.dataSize_);

        //create the const block
        block.constBlock_ = m->renderer_->CreateConstantBlocks(&block.dataSize_, 1);

        //Copy data across to str pool
        hChar* strPtr = block.strPool_;
        strcpy(strPtr/*block.name_*/, block.name_);
        block.name_ = strPtr;
        strPtr += strlen(strPtr)+1;

        for (hUint32 i=0; i < block.aliasCount_; ++i)
        {
            strcpy(strPtr/*block.aliases_[i]*/, block.aliases_[i]);
            block.aliases_[i] = strPtr;
            strPtr += strlen(strPtr)+1;
        }

        for (hUint32 i=0; i < block.paramCount_; ++i)
        {
            strcpy(strPtr/*block.params_[i].name_*/, block.params_[i].name_);
            block.params_[i].name_ = strPtr;
            strPtr += strlen(strPtr)+1;
        }
        hcAssert(strPtr <= block.strPool_+block.strPoolSize_);

        m->constBlocks_.PushBack(block);
        m->constBlockLookUp_.Insert(block.nameHash_, &m->constBlocks_[m->constBlocks_.GetSize()-1]);

        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    int hRenderMaterialManager::RegisterRenderTechnique(lua_State* L)
    {
        hRenderMaterialManager* m = (hRenderMaterialManager*)lua_topointer(L, lua_upvalueindex(1));
        if (!m) luaL_error(L, "Unable to grab engine pointer" );

        m->AddRenderTechnique(luaL_checkstring(L, -1));
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdParameterConstantBlock* hRenderMaterialManager::GetGlobalConstantBlockByAlias(const hChar* name)
    {
        for (hUint32 i = 0, c = constBlocks_.GetSize(); i < c; ++i) {
            for (hUint32 a = 0, ac = constBlocks_[i].aliasCount_; a < ac; ++a) {
                if (hStrCmp(constBlocks_[i].aliases_[a], name) == 0) {
                    return constBlocks_[i].constBlock_;
                }
            }
        }

        return NULL;
    }

}
