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
        matKeys_ = (hMaterialKeyContainer*)hHeapRealloc("general", matKeys_, sizeof(hMaterialKeyContainer)*maxKeys_);
        constBlockLookUp_.SetAutoDelete(false);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderMaterialManager::~hRenderMaterialManager()
    {
        hFreeSafe(matKeys_);
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

    hRenderBuffer* hRenderMaterialManager::GetGlobalConstantBlock(hUint32 id)
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
            matKeys_ = (hMaterialKeyContainer*)hHeapRealloc("general", matKeys_, sizeof(hMaterialKeyContainer)*maxKeys_);
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
        block.strPoolSize_ = (hUint)lua_rawlen(L, -1)+1;// get string len
        lua_pop(L, 1); // remove block.name
        
        lua_getfield(L, -1, "aliases");// push alias table to stack top
        if (lua_type(L, -1) != LUA_TTABLE) luaL_error(L, "block.aliases must be a table type");
        // create space for alias names, we get the pointers to these and copy them later
        block.aliasCount_ = (hUint)lua_rawlen(L,-1);
        block.aliases_ = (const hChar**)hHeapMalloc("general", sizeof(hChar*)*block.aliasCount_);
        block.aliasHashes_ = (hUint32*)hHeapMalloc("general", sizeof(hUint32)*block.aliasCount_);
        for (hUint32 i = 0; i < block.aliasCount_; ++i)
        {
            lua_rawgeti(L, -1, (i+1));// push table entry onto stack (entries start at 1)
            if (lua_type(L, -1) != LUA_TSTRING) luaL_error(L, "alias must be a string");
            block.aliases_[i] = lua_tostring(L, -1);
            block.aliasHashes_[i] = hCRC32::StringCRC(block.aliases_[i]);
            block.strPoolSize_ += (hUint)lua_rawlen(L, -1)+1;// get string len
            lua_pop(L, 1);
        }
        lua_pop(L, 1);// Pop aliases table

        lua_getfield(L, -1, "parameters");// get parameters table
        if (lua_type(L, -1) != LUA_TTABLE) luaL_error(L, "expected parameter table");
        block.paramCount_ = (hUint)lua_rawlen(L, -1);// get parameter count
        block.params_ = (hConstBlockParam*)hHeapMalloc("general", sizeof(hConstBlockParam)*block.paramCount_);
        
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
                block.strPoolSize_ += (hUint)lua_rawlen(L, -1)+1;// get string len
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
        
        //build the string pools
        block.strPool_ = (hChar*)hHeapMalloc("general", block.strPoolSize_);
        block.data_ = hHeapMalloc("general", block.dataSize_);

        //create the const block
        m->renderer_->createBuffer(block.dataSize_, NULL, eResourceFlag_ConstantBuffer, 0, &block.constBlock_);

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

    hRenderBuffer* hRenderMaterialManager::GetGlobalConstantBlockByAlias(const hChar* name)
    {
        return GetGlobalConstantBlockParameterID(hCRC32::StringCRC(name));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderBuffer* hRenderMaterialManager::GetGlobalConstantBlockParameterID(hShaderParameterID id)
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
        debugFontMat_=hNEW(hMaterial)(renderer_);
        hRenderUtility::buildDebugFontMaterial(renderer_, debugFontMat_);
        debugPosColUVMat_=hNEW(hMaterial)(renderer_);
        hRenderUtility::buildDebugPosColUVMaterial(renderer_, debugPosColUVMat_);
        debugPosColMat_=hNEW(hMaterial)(renderer_);
        hRenderUtility::buildDebugPosColMaterial(renderer_, debugPosColMat_);
        debugPosColUVAlphaMat_=hNEW(hMaterial)(renderer_);
        hRenderUtility::buildDebugPosColUVAlphaMaterial(renderer_, debugPosColUVAlphaMat_);
        debugPosColAlphaMat_=hNEW(hMaterial)(renderer_);
        hRenderUtility::buildDebugPosColAlphaMaterial(renderer_, debugPosColAlphaMat_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::destroyRenderResources()
    {

        hDELETE_SAFE(debugFontMat_);
        hDELETE_SAFE(debugPosColUVMat_);
        hDELETE_SAFE(debugPosColMat_);
        hDELETE_SAFE(debugPosColUVAlphaMat_);
        hDELETE_SAFE(debugPosColAlphaMat_);

        for (hUint i = 0, c = constBlocks_.GetSize(); i < c; ++i) {
            constBlocks_[i].constBlock_->DecRef();
            hFreeSafe(constBlocks_[i].aliasHashes_);
            hFreeSafe(constBlocks_[i].aliases_);
            hFreeSafe(constBlocks_[i].strPool_);
            hFreeSafe(constBlocks_[i].data_);
            hFreeSafe(constBlocks_[i].params_);
        }
        constBlockLookUp_.Clear(hFalse);
        constBlocks_.Clear();

        for (hGlobalTexture* i=globalTextures_.GetHead(); i; i=i->GetNext()) {
            hDELETE_ARRAY_SAFE(i->strPool_);
            hDELETE_ARRAY_SAFE(i->aliasHashes_);
            i->texture_->DecRef();
        }
        globalTextures_.Clear(hTrue);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::registerGlobalTexture(const hChar* name, hTexture* tex, const hChar** aliases, hSizeT aliasCount, hBool takeTexture/*=hFalse*/) {
        hcAssert(name && tex && aliases && aliasCount > 0);
        hGlobalTexture* gtex=hNEW(hGlobalTexture);
        hUint strsize;
        hUint len;
        hChar* strptr;
        
        strsize = hStrLen(name)+1;
        for (hUint i=0; i<aliasCount; ++i) {
            strsize += hStrLen(aliases[i])+1;
        }
        strptr=hNEW_ARRAY(hChar, strsize);
        gtex->strPoolSize_=strsize;
        gtex->strPool_=(hChar*)strptr;
        gtex->aliasHashes_=hNEW_ARRAY(hUint32, aliasCount);
        gtex->nameHash_=hCRC32::StringCRC(name);
        gtex->texture_=tex;
        gtex->texture_->AddRef();
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
            renderer_->resizeTexture(width, height, gt->texture_); 
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderMaterialManager::updateGlobalTexture(const hChar* name, hTexture* tex) {
        hcAssert(name && tex);
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
            "rgba32_typeless",
            "rgba32_float",
            "rgba32_uint",
            "rgba32_sint",
            "rgb32_typeless", 
            "rgb32_float",    
            "rgb32_uint",     
            "rgb32_sint",     
            "rgba16_typeless",
            "rgba16_float",   
            "rgba16_unorm",   
            "rgba16_uint",    
            "rgba16_snorm",   
            "rgba16_sint",    
            "rg32_typeless",  
            "rg32_float",     
            "rg32_uint",      
            "rg32_sint",      
            "rg16_typeless",  
            "rg16_float",     
            "rg16_uint",      
            "rg16_sint",      
            "r32_typeless",  
            "r32_float",     
            "r32_uint",      
            "r32_sint",      
            "r16_typeless",  
            "r16_float",     
            "r16_uint",      
            "r16_sint",      
            "rgb10a2_typeless", 
            "rgb10a2_unorm",    
            "rgb10a2_uint",     
            "rgba8_unorm",
            "rgba8_typeless",
            "d32_float",
            "d24S8_float",
            "r8_unorm",
            "bc3_unorm", //dXT5
            "bc2_unorm", //dXT3
            "bc1_unorm", //dXT1
            "rgba8_srgb_unorm",
            "bc3_srgb_unorm",
            "bc2_srgb_unorm",
            "bc1_srgb_unorm",
            NULL
        };
        static const hTextureFormat formats[] = {
            eTextureFormat_RGBA32_typeless,
            eTextureFormat_RGBA32_float,
            eTextureFormat_RGBA32_uint,
            eTextureFormat_RGBA32_sint,
            eTextureFormat_RGB32_typeless, 
            eTextureFormat_RGB32_float,    
            eTextureFormat_RGB32_uint,     
            eTextureFormat_RGB32_sint,     
            eTextureFormat_RGBA16_typeless,
            eTextureFormat_RGBA16_float,   
            eTextureFormat_RGBA16_unorm,   
            eTextureFormat_RGBA16_uint,    
            eTextureFormat_RGBA16_snorm,   
            eTextureFormat_RGBA16_sint,    
            eTextureFormat_RG32_typeless,  
            eTextureFormat_RG32_float,     
            eTextureFormat_RG32_uint,      
            eTextureFormat_RG32_sint,      
            eTextureFormat_RG16_typeless,  
            eTextureFormat_RG16_float,     
            eTextureFormat_RG16_uint,      
            eTextureFormat_RG16_sint,      
            eTextureFormat_R32_typeless,  
            eTextureFormat_R32_float,     
            eTextureFormat_R32_uint,      
            eTextureFormat_R32_sint,      
            eTextureFormat_R16_typeless,  
            eTextureFormat_R16_float,     
            eTextureFormat_R16_uint,      
            eTextureFormat_R16_sint,      
            eTextureFormat_RGB10A2_typeless, 
            eTextureFormat_RGB10A2_unorm,    
            eTextureFormat_RGB10A2_uint,     
            eTextureFormat_RGBA8_unorm,
            eTextureFormat_RGBA8_typeless,
            eTextureFormat_D32_float,
            eTextureFormat_D24S8_float,
            eTextureFormat_R8_unorm,
            eTextureFormat_BC3_unorm, //DXT5
            eTextureFormat_BC2_unorm, //DXT3
            eTextureFormat_BC1_unorm, //DXT1
            eTextureFormat_RGBA8_sRGB_unorm,
            eTextureFormat_BC3_sRGB_unorm  ,
            eTextureFormat_BC2_sRGB_unorm  ,
            eTextureFormat_BC1_sRGB_unorm  ,
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
        aliascount=(hUint)lua_rawlen(L, -1);
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
        lua_pop(L, 1);
        lua_getfield(L, -1, "uav");
        if (lua_type(L, -1) == LUA_TBOOLEAN) {
            if (lua_toboolean(L, -1)) {
                flags |= RESOURCEFLAG_UNORDEREDACCESS;
            }
        }
        lua_pop(L, 1);
        lua_getfield(L, -1, "depth");
        if (lua_type(L, -1) == LUA_TBOOLEAN) {
            if (lua_toboolean(L, -1)) {
                flags |= RESOURCEFLAG_DEPTHTARGET;
            }
        }
        lua_pop(L, 1);
        lua_getfield(L, -1, "rendertarget");
        if (lua_type(L, -1) == LUA_TBOOLEAN) {
            if (lua_toboolean(L, -1)) {
                flags |= RESOURCEFLAG_RENDERTARGET;
            }
        }
        lua_pop(L, 1);
        m->renderer_->createTexture(1, &texdesc, tformat, flags, &texture);
        lua_pop(L, 1);
        m->registerGlobalTexture(name, texture, aliases, aliascount, hTrue);
        //reg global texture takes a ref so dicard our references
        texture->DecRef();
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
        lua_pushnumber(mainLuaState_, (lua_Number)width);
        lua_pushnumber(mainLuaState_, (lua_Number)height);
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
        lua_pushnumber(L, (lua_Number)m->renderer_->GetWidth());
        lua_pushnumber(L, (lua_Number)m->renderer_->GetHeight());
        return 2;
    }
}
