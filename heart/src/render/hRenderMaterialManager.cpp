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

    void hRenderMaterialManager::openLuaMaterialLib(lua_State* L)
    {
        static const luaL_Reg matlib[] = {
            {"registerRenderTechnique", RegisterRenderTechnique},
            {"registerParameterBlock", RegisterParameterBlock},
            {"registerGlobalTexture", registerGlobalTextureLua},
            {"registerWindowResizeCallback", registerResizeLua},
            {"getWindowWidthHeight", getWindowSizeLua},
            {"resizeGlobalTexture", resizeGlobalTextureLua},
            {NULL, NULL}
        };

        lua_getglobal(L, "heart");
        lua_pushlightuserdata(L, this);
        luaL_setfuncs(L,matlib,1);
        lua_pop(L, 1);// pop heart module table

        mainLuaState_=L;
        renderer_->setResizeCallback(hFUNCTOR_BINDMEMBER(hDeviceResizeCallback, hRenderMaterialManager, onWindowResize, this));
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
        block.aliasHashes_ = (hUint32*)hHeapMalloc(GetGlobalHeap(), sizeof(hUint32)*block.aliasCount_);
        for (hUint32 i = 0; i < block.aliasCount_; ++i)
        {
            lua_rawgeti(L, -1, (i+1));// push table entry onto stack (entries start at 1)
            if (lua_type(L, -1) != LUA_TSTRING) luaL_error(L, "alias must be a string");
            block.aliases_[i] = lua_tostring(L, -1);
            block.aliasHashes_[i] = hCRC32::StringCRC(block.aliases_[i]);
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
                block.params_[i].size_ = lua_tounsigned(L, -1);
                block.params_[i].offset_ = (hUint16)block.dataSize_;
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
        block.constBlock_ = m->renderer_->CreateConstantBlocks(&block.dataSize_, NULL, 1);

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
        return GetGlobalConstantBlockParameterID(hCRC32::StringCRC(name));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdParameterConstantBlock* hRenderMaterialManager::GetGlobalConstantBlockParameterID(hShaderParameterID id)
    {
        for (hUint32 i = 0, c = constBlocks_.GetSize(); i < c; ++i) {
            for (hUint32 a = 0, ac = constBlocks_[i].aliasCount_; a < ac; ++a) {
                if (constBlocks_[i].aliasHashes_[a] == id) {
                    return constBlocks_[i].constBlock_;
                }
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::createDebugMaterials() {
        wireframeMat_=hNEW(GetGlobalHeap(), hMaterial)(GetGlobalHeap(), renderer_);
        hRenderUtility::buildDebugWireMaterial(renderer_, wireframeMat_);
        viewLitMat_=hNEW(GetGlobalHeap(), hMaterial)(GetGlobalHeap(), renderer_);
        hRenderUtility::buildDebugViewLitMaterial(renderer_, viewLitMat_);
        consoleMat_=hNEW(GetGlobalHeap(), hMaterial)(GetGlobalHeap(), renderer_);
        hRenderUtility::buildDebugConsoleMaterial(renderer_, consoleMat_);
        debugFontMat_=hNEW(GetGlobalHeap(), hMaterial)(GetGlobalHeap(), renderer_);
        hRenderUtility::buildDebugFontMaterial(renderer_, debugFontMat_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::destroyRenderResources()
    {
        hDELETE_SAFE(GetGlobalHeap(), wireframeMat_);
        hDELETE_SAFE(GetGlobalHeap(), viewLitMat_);
        hDELETE_SAFE(GetGlobalHeap(), consoleMat_);
        hDELETE_SAFE(GetGlobalHeap(), debugFontMat_);

        for (hUint i = 0, c = constBlocks_.GetSize(); i < c; ++i) {
            hHeapFreeSafe(GetGlobalHeap(), constBlocks_[i].aliasHashes_);
            hHeapFreeSafe(GetGlobalHeap(), constBlocks_[i].aliases_);
            hHeapFreeSafe(GetGlobalHeap(), constBlocks_[i].strPool_);
            hHeapFreeSafe(GetGlobalHeap(), constBlocks_[i].data_);
            hHeapFreeSafe(GetGlobalHeap(), constBlocks_[i].params_);
            renderer_->DestroyConstantBlocks(constBlocks_[i].constBlock_, 1);
        }
        constBlockLookUp_.Clear(hFalse);
        constBlocks_.Clear();

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::registerGlobalTexture(const hChar* name, hTexture* tex, const hChar** aliases, hUint aliasCount) {
        hcAssert(name && tex && aliases && aliasCount > 0);
        hGlobalTexture* gtex=hNEW(GetGlobalHeap(), hGlobalTexture);
        hUint strsize;
        hUint len;
        hChar* strptr;
        
        strsize = hStrLen(name)+1;
        for (hUint i=0; i<aliasCount; ++i) {
            strsize += hStrLen(aliases[i])+1;
        }
        strptr=hNEW_ARRAY(GetGlobalHeap(), hChar, strsize);
        gtex->strPoolSize_=strsize;
        gtex->strPool_=(hChar*)strptr;
        gtex->aliasHashes_=hNEW_ARRAY(GetGlobalHeap(), hUint32, aliasCount);
        gtex->nameHash_=hCRC32::StringCRC(name);
        gtex->texture_=tex;
        hStrCopy(strptr, strsize, name);
        len=hStrLen(name)+1;
        strptr+=len;
        strsize-=len;
        for (hUint i=0; i<aliasCount; ++i) {
            gtex->aliasHashes_[i]=hCRC32::StringCRC(aliases[i]);
            hStrCopy(strptr, strsize, aliases[i]);
            len=hStrLen(aliases[i])+1;
            strptr+=len;
            strsize-=len;
        }

        globalTextures_.Insert(gtex->nameHash_, gtex);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::resizeGlobalTexture(const hChar* name, hUint width, hUint height) {
        hGlobalTexture* gt = globalTextures_.Find(hCRC32::StringCRC(name));
        if (gt) {
            hcAssert(gt->texture_);
            hTextureFormat tf=gt->texture_->getTextureFormat();
            hUint32 flags=gt->texture_->getFlags();
            hMipDesc mipdesc={0};

            renderer_->DestroyTexture(gt->texture_);
            mipdesc.width=width;
            mipdesc.height=height;
            renderer_->CreateTexture(width, height, 1, &mipdesc, tf, flags, GetGlobalHeap(), &gt->texture_);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::updateGlobalTexture(const hChar* name, hTexture* tex) {
        hcAssert(name && tex)
        hGlobalTexture* gt = globalTextures_.Find(hCRC32::StringCRC(name));
        if (gt) {
            gt->texture_=tex;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    int hRenderMaterialManager::resizeGlobalTextureLua(lua_State* L) {
        luaL_checkstring(L, 1);
        luaL_checkinteger(L, 2);
        luaL_checkinteger(L, 3);
        hRenderMaterialManager* m = (hRenderMaterialManager*)lua_topointer(L, lua_upvalueindex(1));
        if (!m) {
            luaL_error(L, "Unable to grab engine pointer" );
        }
        m->resizeGlobalTexture(lua_tostring(L, -3), lua_tointeger(L, -2), lua_tointeger(L, -1));
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    int hRenderMaterialManager::registerGlobalTextureLua(lua_State* L) {
        hRenderMaterialManager* m = (hRenderMaterialManager*)lua_topointer(L, lua_upvalueindex(1));
        if (!m) {
            luaL_error(L, "Unable to grab engine pointer" );
        }
        const hChar* name;
        hUint aliascount;
        const hChar** aliases;
        hMipDesc texdesc={0};
        hTextureFormat tformat;
        hTexture* texture;
        static const hChar* formatnames[] = {
            "rgba8"        ,//TFORMAT_ARGB8,
            "rgb8"         ,//TFORMAT_XRGB8,
            "r16_float"    ,//TFORMAT_R16F,
            "rg16_float"   ,//TFORMAT_GR16F,
            "rgba16_float" ,//TFORMAT_ABGR16F,
            "r32_float"    ,//TFORMAT_R32F,
            "d32"          ,//TFORMAT_D32F,
            "d28s8"        ,//TFORMAT_D24S8F,
            "l8"           ,//TFORMAT_L8,
            "dxt5"         ,//TFORMAT_DXT5,
            "dxt3"         ,//TFORMAT_DXT3,
            "dxt1"         ,//TFORMAT_DXT1,
            "rgba8_srgb"   ,//TFORMAT_ARGB8_sRGB
            "rgb8_srgb"    ,//TFORMAT_XRGB8_sRGB
            "dxt5_srgb"    ,//TFORMAT_DXT5_sRGB
            "dxt3_srgb"    ,//TFORMAT_DXT3_sRGB
            "dxt1_srgb"    ,//TFORMAT_DXT1_sRGB
            NULL
        };
        static const hTextureFormat formats[] = {
            TFORMAT_ARGB8,
            TFORMAT_XRGB8,
            TFORMAT_R16F,
            TFORMAT_GR16F,
            TFORMAT_ABGR16F,
            TFORMAT_R32F,
            TFORMAT_D32F,
            TFORMAT_D24S8F,
            TFORMAT_L8,
            TFORMAT_DXT5,
            TFORMAT_DXT3,
            TFORMAT_DXT1,
            TFORMAT_ARGB8_sRGB,
            TFORMAT_XRGB8_sRGB,
            TFORMAT_DXT5_sRGB,
            TFORMAT_DXT3_sRGB,
            TFORMAT_DXT1_sRGB,
        };

        luaL_checktype(L, -1, LUA_TTABLE);
        lua_getfield(L, -1, "name");
        if (lua_type(L, -1) != LUA_TSTRING) {
            luaL_error(L, "globaltexture.name expected string type");
        }
        name=lua_tostring(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "aliases");
        if(lua_type(L, -1) != LUA_TTABLE) {
            luaL_error(L, "globaltexture.aliases expected a table type");
        }
        aliascount=lua_rawlen(L, -1);
        aliases=(const hChar**)hAlloca(sizeof(hChar*)*aliascount);
        for (hUint i=0; i<aliascount; ++i) {
            lua_rawgeti(L, -1, (i+1));// push table entry onto stack (entries start at 1)
            if (lua_type(L, -1) != LUA_TSTRING) {
                luaL_error(L, "alias must be a string");
            }
            aliases[i]=lua_tostring(L, -1);
            lua_pop(L, 1); //pop string
        }
        lua_pop(L, 1);//pop alias table

        lua_getfield(L, -1, "width");
        luaL_checkinteger(L, -1);
        texdesc.width = lua_tointeger(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "height");
        luaL_checkinteger(L, -1);
        texdesc.height = lua_tointeger(L, -1);
        lua_pop(L, 1);
        lua_getfield(L, -1, "format");
        tformat=formats[luaL_checkoption(L, -1, NULL, formatnames)];
        hUint32 flags=0;
        if (tformat == TFORMAT_D24S8F || tformat == TFORMAT_D32F) {
            flags |= RESOURCEFLAG_DEPTHTARGET;
        } else {
            flags |= RESOURCEFLAG_RENDERTARGET;
        }
         m->renderer_->CreateTexture(texdesc.width, texdesc.height, 1, &texdesc, tformat, flags, GetGlobalHeap(), &texture);
        lua_pop(L, 2);
        m->registerGlobalTexture(name, texture, aliases, aliascount);
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hTexture* hRenderMaterialManager::getGlobalTexture(const hChar* name) {
        hGlobalTexture* gt = globalTextures_.Find(hCRC32::StringCRC(name));
        return gt ? gt->texture_ : NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hTexture* hRenderMaterialManager::getGlobalTextureByAlias(const hChar* alias) {
        hUint32 id=hCRC32::StringCRC(alias);
        for (hGlobalTexture* i=globalTextures_.GetHead(); i; i=i->GetNext()) {
            for (hUint32 a = 0, ac = i->aliasCount_; a < ac; ++a) {
                if (i->aliasHashes_[a] == id) {
                    return i->texture_;
                }
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    int hRenderMaterialManager::registerResizeLua(lua_State* L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX, "_hgfx_resize");
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::onWindowResize(hUint width, hUint height) {
        lua_getfield(mainLuaState_, LUA_REGISTRYINDEX, "_hgfx_resize");
        lua_pushnumber(mainLuaState_, width);
        lua_pushnumber(mainLuaState_, height);
        lua_pcall(mainLuaState_, 2, 0, 0);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    int hRenderMaterialManager::getWindowSizeLua(lua_State* L) {
        hRenderMaterialManager* m = (hRenderMaterialManager*)lua_topointer(L, lua_upvalueindex(1));
        if (!m) {
            luaL_error(L, "Unable to grab engine pointer" );
        }
        lua_pushnumber(L, m->renderer_->GetWidth());
        lua_pushnumber(L, m->renderer_->GetHeight());
        return 2;
    }
}
