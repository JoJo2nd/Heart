/********************************************************************

    filename: 	hRenderUtility.cpp	
    
    Copyright (c) 1:4:2012 James Moran
    
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

extern const float g_debugfontHeight;
extern const unsigned int g_debugpageCount;
extern const unsigned int g_debugglyphCount;
extern const Heart::hFontCharacter g_debugglyphs[];
extern const size_t low_res_fnt_width;
extern const size_t low_res_fnt_height;
extern const size_t low_res_fnt_pitch;
extern const size_t low_res_fnt_data_len;
extern const unsigned char low_res_fnt_data[];

namespace Heart
{
namespace hRenderUtility
{

    hFloat ComputeGaussianCurve( hFloat step, hFloat power )
    {
        hFloat theta = power;

        return (hFloat)( (1.0 / sqrt( 2 * HEART_PI * theta ) ) * exp(-(step * step) / (2 * theta * theta)));
    }

    void ComputeBlurWeights( hFloat blurPower, hUint32 samples, hFloat* pOutWeights )
    {
        // The first sample always has a zero offset.
        pOutWeights[0] = ComputeGaussianCurve( 0.0f, blurPower );

        // Maintain a sum of all the weighting values.
        float totalWeights = pOutWeights[0];

        // Add pairs of additional sample taps, positioned
        // along a line in both directions from the center.
        for ( hUint32 i = 0; i < samples / 2; ++i )
        {
            // Store weights for the positive and negative taps.
            hFloat weight = ComputeGaussianCurve( i + 1.0f, blurPower );

            pOutWeights[i * 2 + 1] = weight;
            pOutWeights[i * 2 + 2] = weight;

            totalWeights += weight * 2;
        }

        // Normalize the list of sample weightings, so they will always sum to one.
        for (hUint32 i = 0; i < samples; i++)
        {
            pOutWeights[i] /= totalWeights;
        }
    }

    void ComputeBlurOffsets( hFloat dx, hFloat dy, hUint32 samples, hVec2* pOutSamples )
    {
        // The first sample always has a zero offset.
        pOutSamples[0] = hVec2Func::zeroVector();

        // Add pairs of additional sample taps, positioned
        // along a line in both directions from the center.
        for ( hUint32 i = 0; i < samples / 2; ++i )
        {
            // To get the maximum amount of blurring from a limited number of
            // pixel shader samples, we take advantage of the bilinear filtering
            // hardware inside the texture fetch unit. If we position our texture
            // coordinates exactly halfway between two texels, the filtering unit
            // will average them for us, giving two samples for the price of one.
            // This allows us to step in units of two texels per sample, rather
            // than just one at a time. The 1.5 offset kicks things off by
            // positioning us nicely in between two texels.
            hFloat sampleOffset = i * 2 + 1.5f;

            hVec2 delta = hVec2( dx, dy ) * sampleOffset;

            // Store texture coordinate offsets for the positive and negative taps.
            pOutSamples[i * 2 + 1] = delta;
            pOutSamples[i * 2 + 2] = -delta;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    void HEART_API buildSphereMesh(hUint16 segments, hUint16 rings, hFloat radius, hRenderer* rndr, 
    hIndexBuffer** outIdxBuf, hVertexBuffer** outVtxBuf)
    {
        hInputLayoutDesc desc[] = {
            hInputLayoutDesc("POSITION", 0, eIF_FLOAT3, 0, 0),
            hInputLayoutDesc("NORMAL", 0, eIF_FLOAT3, 0, 0),
        };
        hUint16 vtxCnt = GetSphereMeshVertexCount( segments, rings );
        hUint16 idxCnt = GetSphereMeshIndexCount( segments, rings );

        hFloat* verts = (hFloat*)hAlloca(vtxCnt*6*sizeof(hFloat));
        hUint16* indices = (hUint16*)hAlloca(idxCnt*sizeof(hUint16));
        hFloat* vtx  = verts;
        hUint16* idx = indices;

        hFloat dRingAngle = (HEART_PI / rings);
        hFloat dSegAngle = (2 * HEART_PI / segments);

        hUint16 idxIdx = 0;
        hUint16 vtxIdx = 0;

        // Generate the group of rings for the sphere
        for( hUint32 ring = 0; ring <= rings; ring++ ) 
        {
            float r0 = radius * sinf (ring * dRingAngle);
            float y0 = radius * cosf (ring * dRingAngle);

            // Generate the group of segments for the current ring
            for( hUint32 seg = 0; seg <= segments; seg++) 
            {
                float x0 = r0 * sinf(seg * dSegAngle);
                float z0 = r0 * cosf(seg * dSegAngle);
                hVec3 v( x0, y0, z0 );
                hCPUVec3 vNormal = hVec3Func::normaliseFast( v );//needs flipping, but shouldn't

                // Add one vertex to the strip which makes up the sphere
                *vtx++ = x0;
                *vtx++ = y0;
                *vtx++ = z0;

				//Normal
	 			*vtx++ = vNormal.x;
	 			*vtx++ = vNormal.y;
	 			*vtx++ = vNormal.z;
// 
// 				//UV
// 	 			*vtx++ = (float) seg / (float) segments;
// 	 			*vtx++ = (float) ring / (float) rings;

                if ( ring != rings ) 
                {
                    // each vertex (except the last) has six indices pointing to it
                    *idx++ = vtxIdx + segments + 1;
                    *idx++ = vtxIdx;               
                    *idx++ = vtxIdx + segments;
                    *idx++ = vtxIdx + segments + 1;
                    *idx++ = vtxIdx + 1;
                    *idx++ = vtxIdx;
                }
                ++vtxIdx;
            }
        }

        rndr->createIndexBuffer(indices, idxCnt, 0, outIdxBuf);
        rndr->createVertexBuffer(verts, vtxCnt, desc, (hUint32)hStaticArraySize(desc), 0, outVtxBuf);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    extern void BuildFullscreenQuadMesh( 
        hFloat width,
        hFloat height,
        hRenderSubmissionCtx* ctx,  
        hIndexBuffer* idxBuf, 
        hVertexBuffer* vtxBuf )
    {
        hUint16 quadIdx[] =
        {
            0,2,1,2,3,1
        };
        hFloat wo2 = width / 2.0f;
        hFloat ho2 = height / 2.0f;
        struct Vertex
        {
            hCPUVec3 pos;
            hCPUVec2 uv;
        };

        hIndexBufferMapInfo ibMapInfo;
        ctx->Map( idxBuf, &ibMapInfo );

        hMemCpy( ibMapInfo.ptr_, quadIdx, sizeof(quadIdx) );

        ctx->Unmap( &ibMapInfo );

        hVertexBufferMapInfo vbMapInfo;
        ctx->Map( vtxBuf, &vbMapInfo );

        Vertex* vtx = (Vertex*)vbMapInfo.ptr_;

        vtx[0].pos = hCPUVec3( -wo2, ho2,  0.25f );
        vtx[0].uv  = hCPUVec2( 0.f, 0.f );

        vtx[0].pos = hCPUVec3(  wo2, ho2,  0.25f );
        vtx[0].uv  = hCPUVec2( 1.f, 0.f );

        vtx[0].pos = hCPUVec3( -wo2, -ho2, 0.25f );
        vtx[0].uv  = hCPUVec2( 0.f, 1.f );

        vtx[0].pos = hCPUVec3(  wo2, -ho2, 0.25f );
        vtx[0].uv  = hCPUVec2( 1.f, 1.f );

        ctx->Unmap( &vbMapInfo );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT 
    void HEART_API buildTessellatedQuadMesh(hFloat width, hFloat height, hUint wdivs, hUint hdivs, 
    hRenderer* rndr, hIndexBuffer** outIdxBuf, hVertexBuffer** outVtxBuf) {
        hcAssert(width > 0.f && height > 0.f && wdivs >= 1 && hdivs >= 1);
        hcAssert(outVtxBuf && rndr);
        hInputLayoutDesc desc[] = {
            hInputLayoutDesc("POSITION", 0, eIF_FLOAT3, 0, 0),
            hInputLayoutDesc("TEXCOORD", 0, eIF_FLOAT2, 0, 0),
        };
        hFloat uvxstep=1.f/wdivs;
        hFloat uvystep=-1.f/hdivs;
        hFloat xstep=width/wdivs;
        hFloat ystep=height/hdivs;
        hFloat xc=0.f;
        hFloat yc=0.f;
        hFloat u=0.f;
        hFloat v=1.f;
        hUint16 currIdx=0;

        void* verts=hAlloca(sizeof(hFloat)*5*(wdivs+1)*(hdivs+1));
        void* index=hAlloca(sizeof(hUint16)*6*(wdivs)*(hdivs));
        hFloat* vtx=(hFloat*)verts;
        hUint16* idx=(hUint16*)index;

        for (hUint y=0,yn=(hdivs+1); y<yn; ++y, yc+=ystep, v+=uvystep) {
            xc=0.f;
            u=0.f;
            for (hUint x=0,xn=(wdivs+1); x<xn; ++x, xc+=xstep, u+=uvxstep) {
                vtx[0]=xc;vtx[1]=yc;vtx[2]=0.f;
                vtx[3]=u; vtx[4]=v;
                vtx+=5;
            }
        }
        for (hUint y=0,yn=(hdivs); y<yn; ++y) {
            for (hUint x=0,xn=(wdivs); x<xn; ++x) {
                hUint16 currIdx=(y*(wdivs+1))+x;
                idx[0]=currIdx;
                idx[1]=currIdx+1;
                idx[2]=currIdx+wdivs+1;
                idx[3]=currIdx+1;
                idx[4]=currIdx+wdivs+2;
                idx[5]=currIdx+wdivs+1;
                idx+=6;
            }
        }

        rndr->createIndexBuffer(index, 6*(wdivs)*(hdivs), 0, outIdxBuf);
        rndr->createVertexBuffer(verts, (wdivs+1)*(hdivs+1), desc, (hUint32)hStaticArraySize(desc), 0, outVtxBuf);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT 
    void HEART_API buildConeMesh(hUint16 segments, hFloat radius, hFloat depth, hRenderer* rndr, hVertexBuffer** outVtxBuf)
    {
        hcAssert(outVtxBuf && rndr);
        hInputLayoutDesc desc[] = {
            hInputLayoutDesc("POSITION", 0, eIF_FLOAT3, 0, 0),
            hInputLayoutDesc("NORMAL", 0, eIF_FLOAT3, 0, 0),
        };
        struct Vertex
        {
            hCPUVec3 pos;
            hCPUVec3 normal;
        };

        segments = hMax(segments, 4);
        hUint16 vtxCnt = (hUint16)(segments*6);
        hUint16 iidx = 0;
        hUint16 vidx = 0;
        void* verts=hAlloca(sizeof(Vertex)*vtxCnt);
        Vertex* vtx = (Vertex*)verts;
        hFloat dSegAngle = (2 * HEART_PI / segments);

        hVec3 conetip(0.f, 0.f, 0.f);
        hVec3 conebase(0.f, 0.f, depth);

        //Create the Cone
        for ( hUint16 i = 0; i < segments; ++i )
        {
            float x1 = radius * sinf( dSegAngle*i );
            float y1 = radius * cosf( dSegAngle*i );
            float z1 = depth;
            float x2 = radius * sinf( dSegAngle*(i+1) );
            float y2 = radius * cosf( dSegAngle*(i+1) );
            float z2 = depth;

            hVec3 v1(x1, y1, z1);
            hVec3 v2=conetip;
            hVec3 v3(x2, y2, z2);
            vtx[0].pos = v1;
            vtx[1].pos = v2;
            vtx[2].pos = v3;
            vtx[0].normal=hVec3Func::cross(v3-v1, v2-v1);
            vtx[1].normal=hVec3Func::cross(v1-v2, v3-v2);
            vtx[2].normal=hVec3Func::cross(v2-v3, v1-v3);
            vtx+=3;
        }

        //Create the Base
        for ( hUint16 i = 0; i < segments; ++i )
        {
            float x1 = radius * sinf( dSegAngle*i );
            float y1 = radius * cosf( dSegAngle*i );
            float z1 = depth;
            float x2 = radius * sinf( dSegAngle*(i+1) );
            float y2 = radius * cosf( dSegAngle*(i+1) );
            float z2 = depth;

            hVec3 v1=conebase;
            hVec3 v2(x1, y1, z1);
            hVec3 v3(x2, y2, z2);
            vtx[0].pos = v1;
            vtx[1].pos = v2;
            vtx[2].pos = v3;
            vtx[0].normal=hVec3Func::cross(v3-v1, v2-v1);
            vtx[1].normal=hVec3Func::cross(v1-v2, v3-v2);
            vtx[2].normal=hVec3Func::cross(v2-v3, v1-v3);
            vtx+=3;
        }

        rndr->createVertexBuffer(verts, vtxCnt, desc, (hUint32)hStaticArraySize(desc), 0, outVtxBuf);
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

#define PRIM_COUNT (36)
    hVertexBuffer* buildDebugCubeMesh(hRenderer* rndr, hVertexBuffer** retVB) {
        hInputLayoutDesc desc[] = {
            hInputLayoutDesc("POSITION", 0, eIF_FLOAT3, 0, 0),
            hInputLayoutDesc("NORMAL", 0, eIF_FLOAT3, 0, 0),
        };
        hFloat verts[PRIM_COUNT*(3+3)] = {
            // Positions        Normals
            // front            
            -.5f, -.5f, -.5f,   0.f, 0.f, -1.f,
            -.5f,  .5f, -.5f,   0.f, 0.f, -1.f,
             .5f,  .5f, -.5f,   0.f, 0.f, -1.f,

            -.5f, -.5f, -.5f,   0.f, 0.f, -1.f,
             .5f,  .5f, -.5f,   0.f, 0.f, -1.f,
             .5f, -.5f, -.5f,   0.f, 0.f, -1.f,

            //top
            -.5f,  .5f, -.5f,   0.f, 1.f, 0.f,
            -.5f,  .5f,  .5f,   0.f, 1.f, 0.f,
             .5f,  .5f,  .5f,   0.f, 1.f, 0.f,
                                          
            -.5f,  .5f, -.5f,   0.f, 1.f, 0.f,
             .5f,  .5f,  .5f,   0.f, 1.f, 0.f,
             .5f,  .5f, -.5f,   0.f, 1.f, 0.f,

            //left
            -.5f, -.5f, -.5f,   -1.f, 0.f, 0.f,
            -.5f,  .5f, -.5f,   -1.f, 0.f, 0.f,
            -.5f,  .5f,  .5f,   -1.f, 0.f, 0.f,
                                        
            -.5f, -.5f, -.5f,   -1.f, 0.f, 0.f,
            -.5f,  .5f,  .5f,   -1.f, 0.f, 0.f,
            -.5f, -.5f,  .5f,   -1.f, 0.f, 0.f,
            
            //right
            .5f, -.5f, -.5f,   1.f, 0.f, 0.f,
            .5f,  .5f, -.5f,   1.f, 0.f, 0.f,
            .5f,  .5f,  .5f,   1.f, 0.f, 0.f,
                                         
            .5f, -.5f, -.5f,   1.f, 0.f, 0.f,
            .5f,  .5f,  .5f,   1.f, 0.f, 0.f,
            .5f, -.5f,  .5f,   1.f, 0.f, 0.f,

            //bottom
            -.5f, -.5f, -.5f,   0.f, -1.f,  0.f,
            -.5f, -.5f,  .5f,   0.f, -1.f,  0.f,
             .5f, -.5f,  .5f,   0.f, -1.f,  0.f,
                                          
            -.5f, -.5f, -.5f,   0.f, -1.f,  0.f,
             .5f, -.5f,  .5f,   0.f, -1.f,  0.f,
             .5f, -.5f, -.5f,   0.f, -1.f,  0.f,

            // back
            -.5f, -.5f,  .5f,   0.f, 0.f, 1.f,
            -.5f,  .5f,  .5f,   0.f, 0.f, 1.f,
             .5f,  .5f,  .5f,   0.f, 0.f, 1.f,

            -.5f, -.5f,  .5f,   0.f, 0.f, 1.f,
             .5f,  .5f,  .5f,   0.f, 0.f, 1.f,
             .5f, -.5f,  .5f,   0.f, 0.f, 1.f,
        };
        rndr->createVertexBuffer(verts, PRIM_COUNT, desc, (hUint32)hStaticArraySize(desc), 0, retVB);
        return *retVB;
    }
#undef PRIM_COUNT

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hMaterial* HEART_API buildDebugFontMaterial(hRenderer* rndr, hResourceManager* resmana, hMaterial* ddrawmat) {
        hcAssert(rndr && ddrawmat);

        
        hBlendStateDesc blendDesc;
        blendDesc.blendEnable_           = RSV_ENABLE;
        blendDesc.srcBlend_              = RSV_BLEND_OP_SRC_ALPHA;
        blendDesc.destBlend_             = RSV_BLEND_OP_INVSRC_ALPHA;
        blendDesc.blendOp_               = RSV_BLEND_FUNC_ADD;
        blendDesc.srcBlendAlpha_         = RSV_BLEND_OP_ONE;
        blendDesc.destBlendAlpha_        = RSV_BLEND_OP_ZERO;
        blendDesc.blendOpAlpha_          = RSV_BLEND_FUNC_ADD;
        blendDesc.renderTargetWriteMask_ = RSV_COLOUR_WRITE_FULL;
        hBlendState* bs=rndr->createBlendState(blendDesc);
        hDepthStencilStateDesc depthDesc;
        depthDesc.depthEnable_         = RSV_ENABLE;       //
        depthDesc.depthWriteMask_      = RSV_DISABLE;       //
        depthDesc.depthFunc_           = RSV_Z_CMP_LESS;   //
        depthDesc.stencilEnable_       = RSV_DISABLE;      //
        depthDesc.stencilReadMask_     = ~0U;              //
        depthDesc.stencilWriteMask_    = ~0U;              //
        depthDesc.stencilFailOp_       = RSV_SO_KEEP;      //
        depthDesc.stencilDepthFailOp_  = RSV_SO_KEEP;      //
        depthDesc.stencilPassOp_       = RSV_SO_KEEP;      //
        depthDesc.stencilFunc_         = RSV_SF_CMP_ALWAYS;//
        depthDesc.stencilRef_          = 0;                //
        hDepthStencilState* ds=rndr->createDepthStencilState(depthDesc);
        hRasterizerStateDesc rastDesc;
        rastDesc.fillMode_              = RSV_FILL_MODE_SOLID;        //
        rastDesc.cullMode_              = RSV_CULL_MODE_NONE;         //
        rastDesc.frontCounterClockwise_ = RSV_ENABLE;                 //
        rastDesc.depthBias_             = 0;                          //
        rastDesc.depthBiasClamp_        = 0.f;                        //
        rastDesc.slopeScaledDepthBias_  = 0.f;                        //
        rastDesc.depthClipEnable_       = RSV_ENABLE;                 //
        rastDesc.scissorEnable_         = RSV_DISABLE;                //
        hRasterizerState* rs=rndr->createRasterizerState(rastDesc);

        hMaterialGroup* group = ddrawmat->addGroup("debug");
        hMaterialTechnique* tech = group->addTechnique("main");
        tech->SetMask(rndr->GetMaterialManager()->AddRenderTechnique("main")->mask_);
        tech->SetSort(hFalse);
        tech->SetLayer(0);
        tech->SetPasses(1);
        hMaterialTechniquePass* pass=tech->appendPass();
        pass->bindBlendState(bs);
        bs->DecRef();
        pass->bindDepthStencilState(ds);
        ds->DecRef();
        pass->bindRasterizerState(rs);
        rs->DecRef();
        pass->setProgramID(ShaderType_VERTEXPROG, hDebugShaderResourceID_FontVertex);
        pass->setProgramID(ShaderType_FRAGMENTPROG, hDebugShaderResourceID_FontPixel);

        hSamplerStateDesc sampDesc;
        sampDesc.filter_        = SSV_POINT;
        sampDesc.addressU_      = SSV_CLAMP;
        sampDesc.addressV_      = SSV_CLAMP;
        sampDesc.addressW_      = SSV_CLAMP;
        sampDesc.mipLODBias_    = 0;
        sampDesc.maxAnisotropy_ = 16;
        sampDesc.borderColour_  = WHITE;
        sampDesc.minLOD_        = -FLT_MAX;
        sampDesc.maxLOD_        = FLT_MAX;
        hSamplerState* ss=rndr->createSamplerState(sampDesc);
        //ddrawmat->bindSampler(hCRC32::StringCRC("g_sampler"), ss);
        hSamplerParameter ssparam;
        ssparam.init("g_sampler", ss);
        ddrawmat->addSamplerParameter(ssparam);
        //ss->DecRef();

        ddrawmat->addDefaultParameterValue("g_sampler", hStringID("?builtin/debug_font_surface"));
        //ddrawmat->setResourceID(hStringID("?builtin/debug_font_material"));
        ddrawmat->listenToResourceEvents(resmana);

        return ddrawmat;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hMaterial* HEART_API buildDebugPosColUVMaterial(hRenderer* rndr, hResourceManager* resmana, hMaterial* ddrawmat) {
        hcAssert(rndr && ddrawmat);

        hBlendStateDesc blendDesc;
        blendDesc.blendEnable_           = RSV_DISABLE;
        blendDesc.srcBlend_              = RSV_BLEND_OP_SRC_ALPHA;
        blendDesc.destBlend_             = RSV_BLEND_OP_INVSRC_ALPHA;
        blendDesc.blendOp_               = RSV_BLEND_FUNC_ADD;
        blendDesc.srcBlendAlpha_         = RSV_BLEND_OP_ONE;
        blendDesc.destBlendAlpha_        = RSV_BLEND_OP_ZERO;
        blendDesc.blendOpAlpha_          = RSV_BLEND_FUNC_ADD;
        blendDesc.renderTargetWriteMask_ = RSV_COLOUR_WRITE_FULL;
        hBlendState* bs=rndr->createBlendState(blendDesc);
        hDepthStencilStateDesc depthDesc;
        depthDesc.depthEnable_         = RSV_ENABLE;       //
        depthDesc.depthWriteMask_      = RSV_DISABLE;       //
        depthDesc.depthFunc_           = RSV_Z_CMP_LESS;   //
        depthDesc.stencilEnable_       = RSV_DISABLE;      //
        depthDesc.stencilReadMask_     = ~0U;              //
        depthDesc.stencilWriteMask_    = ~0U;              //
        depthDesc.stencilFailOp_       = RSV_SO_KEEP;      //
        depthDesc.stencilDepthFailOp_  = RSV_SO_KEEP;      //
        depthDesc.stencilPassOp_       = RSV_SO_KEEP;      //
        depthDesc.stencilFunc_         = RSV_SF_CMP_ALWAYS;//
        depthDesc.stencilRef_          = 0;                //
        hDepthStencilState* ds=rndr->createDepthStencilState(depthDesc);
        hRasterizerStateDesc rastDesc;
        rastDesc.fillMode_              = RSV_FILL_MODE_SOLID;        //
        rastDesc.cullMode_              = RSV_CULL_MODE_NONE;         //
        rastDesc.frontCounterClockwise_ = RSV_ENABLE;                 //
        rastDesc.depthBias_             = 0;                          //
        rastDesc.depthBiasClamp_        = 0.f;                        //
        rastDesc.slopeScaledDepthBias_  = 0.f;                        //
        rastDesc.depthClipEnable_       = RSV_ENABLE;                 //
        rastDesc.scissorEnable_         = RSV_DISABLE;                //
        hRasterizerState* rs=rndr->createRasterizerState(rastDesc);
        hSamplerStateDesc sampDesc;
        sampDesc.filter_        = SSV_LINEAR;
        sampDesc.addressU_      = SSV_CLAMP;
        sampDesc.addressV_      = SSV_CLAMP;
        sampDesc.addressW_      = SSV_CLAMP;
        sampDesc.mipLODBias_    = 0;
        sampDesc.maxAnisotropy_ = 16;
        sampDesc.borderColour_  = WHITE;
        sampDesc.minLOD_        = -FLT_MAX;
        sampDesc.maxLOD_        = FLT_MAX;  

        hMaterialGroup* group = ddrawmat->addGroup("debug");
        hMaterialTechnique* tech = group->addTechnique("main");
        tech->SetMask(rndr->GetMaterialManager()->AddRenderTechnique("main")->mask_);
        tech->SetSort(hFalse);
        tech->SetLayer(0);
        tech->SetPasses(1);
        hMaterialTechniquePass* pass=tech->appendPass();
        pass->bindBlendState(bs);
        bs->DecRef();
        pass->bindDepthStencilState(ds);
        ds->DecRef();
        pass->bindRasterizerState(rs);
        rs->DecRef();
        pass->setProgramID(ShaderType_VERTEXPROG, hDebugShaderResourceID_TexVertex);
        pass->setProgramID(ShaderType_FRAGMENTPROG, hDebugShaderResourceID_TexPixel);

        hSamplerState* ss=rndr->createSamplerState(sampDesc);
        hSamplerParameter ssparam;
        ssparam.init("g_sampler", ss);
        ddrawmat->addSamplerParameter(ssparam);
        //ddrawmat->bindSampler(hCRC32::StringCRC("g_sampler"), ss);
        //ss->DecRef();

        //ddrawmat->setResourceID(hResourceID::buildResourceID("?builtin.debug_pos_col_uv_material"));
        ddrawmat->listenToResourceEvents(resmana);

        return ddrawmat;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hMaterial* HEART_API buildDebugPosColMaterial(hRenderer* rndr, hResourceManager* resmana, hMaterial* ddrawmat) {
        hcAssert(rndr && ddrawmat);

        hBlendStateDesc blendDesc;
        blendDesc.blendEnable_           = RSV_DISABLE;
        blendDesc.srcBlend_              = RSV_BLEND_OP_SRC_ALPHA;
        blendDesc.destBlend_             = RSV_BLEND_OP_INVSRC_ALPHA;
        blendDesc.blendOp_               = RSV_BLEND_FUNC_ADD;
        blendDesc.srcBlendAlpha_         = RSV_BLEND_OP_ONE;
        blendDesc.destBlendAlpha_        = RSV_BLEND_OP_ZERO;
        blendDesc.blendOpAlpha_          = RSV_BLEND_FUNC_ADD;
        blendDesc.renderTargetWriteMask_ = RSV_COLOUR_WRITE_FULL;
        hBlendState* bs=rndr->createBlendState(blendDesc);
        hDepthStencilStateDesc depthDesc;
        depthDesc.depthEnable_         = RSV_ENABLE;       //
        depthDesc.depthWriteMask_      = RSV_ENABLE;       //
        depthDesc.depthFunc_           = RSV_Z_CMP_LESS;   //
        depthDesc.stencilEnable_       = RSV_DISABLE;      //
        depthDesc.stencilReadMask_     = ~0U;              //
        depthDesc.stencilWriteMask_    = ~0U;              //
        depthDesc.stencilFailOp_       = RSV_SO_KEEP;      //
        depthDesc.stencilDepthFailOp_  = RSV_SO_KEEP;      //
        depthDesc.stencilPassOp_       = RSV_SO_KEEP;      //
        depthDesc.stencilFunc_         = RSV_SF_CMP_ALWAYS;//
        depthDesc.stencilRef_          = 0;                //
        hDepthStencilState* ds=rndr->createDepthStencilState(depthDesc);
        hRasterizerStateDesc rastDesc;
        rastDesc.fillMode_              = RSV_FILL_MODE_SOLID;        //
        rastDesc.cullMode_              = RSV_CULL_MODE_NONE;         //
        rastDesc.frontCounterClockwise_ = RSV_ENABLE;                 //
        rastDesc.depthBias_             = 0;                          //
        rastDesc.depthBiasClamp_        = 0.f;                        //
        rastDesc.slopeScaledDepthBias_  = 0.f;                        //
        rastDesc.depthClipEnable_       = RSV_ENABLE;                 //
        rastDesc.scissorEnable_         = RSV_DISABLE;                //
        hRasterizerState* rs=rndr->createRasterizerState(rastDesc);

        hMaterialGroup* group = ddrawmat->addGroup("debug");
        hMaterialTechnique* tech = group->addTechnique("main");
        tech->SetMask(rndr->GetMaterialManager()->AddRenderTechnique("main")->mask_);
        tech->SetSort(hFalse);
        tech->SetLayer(0);
        tech->SetPasses(1);
        hMaterialTechniquePass* pass=tech->appendPass();
        pass->bindBlendState(bs);
        bs->DecRef();
        pass->bindDepthStencilState(ds);
        ds->DecRef();
        pass->bindRasterizerState(rs);
        rs->DecRef();
        pass->setProgramID(ShaderType_VERTEXPROG, hDebugShaderResourceID_VertexPosCol);
        pass->setProgramID(ShaderType_FRAGMENTPROG, hDebugShaderResourceID_PixelPosCol);

        //ddrawmat->setResourceID(hResourceID::buildResourceID("?builtin.debug_pos_col_material"));
        ddrawmat->listenToResourceEvents(resmana);

        return ddrawmat;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hMaterial* HEART_API buildDebugPosColUVAlphaMaterial(hRenderer* rndr, hResourceManager* resmana, hMaterial* ddrawmat) {
        hcAssert(rndr && ddrawmat);

        hBlendStateDesc blendDesc;
        blendDesc.blendEnable_           = RSV_DISABLE;
        blendDesc.srcBlend_              = RSV_BLEND_OP_SRC_ALPHA;
        blendDesc.destBlend_             = RSV_BLEND_OP_INVSRC_ALPHA;
        blendDesc.blendOp_               = RSV_BLEND_FUNC_ADD;
        blendDesc.srcBlendAlpha_         = RSV_BLEND_OP_ONE;
        blendDesc.destBlendAlpha_        = RSV_BLEND_OP_ZERO;
        blendDesc.blendOpAlpha_          = RSV_BLEND_FUNC_ADD;
        blendDesc.renderTargetWriteMask_ = RSV_COLOUR_WRITE_FULL;
        hBlendState* bs=rndr->createBlendState(blendDesc);
        hDepthStencilStateDesc depthDesc;
        depthDesc.depthEnable_         = RSV_ENABLE;       //
        depthDesc.depthWriteMask_      = RSV_DISABLE;       //
        depthDesc.depthFunc_           = RSV_Z_CMP_LESS;   //
        depthDesc.stencilEnable_       = RSV_DISABLE;      //
        depthDesc.stencilReadMask_     = ~0U;              //
        depthDesc.stencilWriteMask_    = ~0U;              //
        depthDesc.stencilFailOp_       = RSV_SO_KEEP;      //
        depthDesc.stencilDepthFailOp_  = RSV_SO_KEEP;      //
        depthDesc.stencilPassOp_       = RSV_SO_KEEP;      //
        depthDesc.stencilFunc_         = RSV_SF_CMP_ALWAYS;//
        depthDesc.stencilRef_          = 0;                //
        hDepthStencilState* ds=rndr->createDepthStencilState(depthDesc);
        hRasterizerStateDesc rastDesc;
        rastDesc.fillMode_              = RSV_FILL_MODE_SOLID;        //
        rastDesc.cullMode_              = RSV_CULL_MODE_NONE;         //
        rastDesc.frontCounterClockwise_ = RSV_ENABLE;                 //
        rastDesc.depthBias_             = 0;                          //
        rastDesc.depthBiasClamp_        = 0.f;                        //
        rastDesc.slopeScaledDepthBias_  = 0.f;                        //
        rastDesc.depthClipEnable_       = RSV_ENABLE;                 //
        rastDesc.scissorEnable_         = RSV_DISABLE;                //
        hRasterizerState* rs=rndr->createRasterizerState(rastDesc);
        hSamplerStateDesc sampDesc;
        sampDesc.filter_        = SSV_LINEAR;
        sampDesc.addressU_      = SSV_CLAMP;
        sampDesc.addressV_      = SSV_CLAMP;
        sampDesc.addressW_      = SSV_CLAMP;
        sampDesc.mipLODBias_    = 0;
        sampDesc.maxAnisotropy_ = 16;
        sampDesc.borderColour_  = WHITE;
        sampDesc.minLOD_        = -FLT_MAX;
        sampDesc.maxLOD_        = FLT_MAX;  

        hMaterialGroup* group = ddrawmat->addGroup("debug");
        hMaterialTechnique* tech = group->addTechnique("main");
        tech->SetMask(rndr->GetMaterialManager()->AddRenderTechnique("main")->mask_);
        tech->SetSort(hFalse);
        tech->SetLayer(0);
        tech->SetPasses(1);
        hMaterialTechniquePass* pass=tech->appendPass();
        pass->bindBlendState(bs);
        bs->DecRef();
        pass->bindDepthStencilState(ds);
        ds->DecRef();
        pass->bindRasterizerState(rs);
        rs->DecRef();
        pass->setProgramID(ShaderType_VERTEXPROG, hDebugShaderResourceID_TexVertex);
        pass->setProgramID(ShaderType_FRAGMENTPROG, hDebugShaderResourceID_TexPixel);

        //ddrawmat->bind();
        hSamplerState* ss=rndr->createSamplerState(sampDesc);
        hSamplerParameter ssparam;
        ssparam.init("g_sampler", ss);
        ddrawmat->addSamplerParameter(ssparam);
        //ddrawmat->bindSampler(hCRC32::StringCRC("g_sampler"), ss);
        //ss->DecRef();

        //ddrawmat->setResourceID(hResourceID::buildResourceID("?builtin.debug_pos_col_uv_alpha_material"));
        ddrawmat->listenToResourceEvents(resmana);

        return ddrawmat;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hMaterial* HEART_API buildDebugPosColAlphaMaterial(hRenderer* rndr, hResourceManager* resmana, hMaterial* ddrawmat) {
        hcAssert(rndr && ddrawmat);

        hBlendStateDesc blendDesc;
        blendDesc.blendEnable_           = RSV_ENABLE;
        blendDesc.srcBlend_              = RSV_BLEND_OP_SRC_ALPHA;
        blendDesc.destBlend_             = RSV_BLEND_OP_INVSRC_ALPHA;
        blendDesc.blendOp_               = RSV_BLEND_FUNC_ADD;
        blendDesc.srcBlendAlpha_         = RSV_BLEND_OP_ONE;
        blendDesc.destBlendAlpha_        = RSV_BLEND_OP_ZERO;
        blendDesc.blendOpAlpha_          = RSV_BLEND_FUNC_ADD;
        blendDesc.renderTargetWriteMask_ = RSV_COLOUR_WRITE_FULL;
        hBlendState* bs=rndr->createBlendState(blendDesc);
        hDepthStencilStateDesc depthDesc;
        depthDesc.depthEnable_         = RSV_ENABLE;       //
        depthDesc.depthWriteMask_      = RSV_DISABLE;       //
        depthDesc.depthFunc_           = RSV_Z_CMP_LESS;   //
        depthDesc.stencilEnable_       = RSV_DISABLE;      //
        depthDesc.stencilReadMask_     = ~0U;              //
        depthDesc.stencilWriteMask_    = ~0U;              //
        depthDesc.stencilFailOp_       = RSV_SO_KEEP;      //
        depthDesc.stencilDepthFailOp_  = RSV_SO_KEEP;      //
        depthDesc.stencilPassOp_       = RSV_SO_KEEP;      //
        depthDesc.stencilFunc_         = RSV_SF_CMP_ALWAYS;//
        depthDesc.stencilRef_          = 0;                //
        hDepthStencilState* ds=rndr->createDepthStencilState(depthDesc);
        hRasterizerStateDesc rastDesc;
        rastDesc.fillMode_              = RSV_FILL_MODE_SOLID;        //
        rastDesc.cullMode_              = RSV_CULL_MODE_NONE;         //
        rastDesc.frontCounterClockwise_ = RSV_ENABLE;                 //
        rastDesc.depthBias_             = 0;                          //
        rastDesc.depthBiasClamp_        = 0.f;                        //
        rastDesc.slopeScaledDepthBias_  = 0.f;                        //
        rastDesc.depthClipEnable_       = RSV_ENABLE;                 //
        rastDesc.scissorEnable_         = RSV_DISABLE;                //
        hRasterizerState* rs=rndr->createRasterizerState(rastDesc);

        hMaterialGroup* group = ddrawmat->addGroup("debug");
        hMaterialTechnique* tech = group->addTechnique("main");
        tech->SetMask(rndr->GetMaterialManager()->AddRenderTechnique("main")->mask_);
        tech->SetSort(hFalse);
        tech->SetLayer(0);
        tech->SetPasses(1);
        hMaterialTechniquePass* pass=tech->appendPass();
        pass->bindBlendState(bs);
        bs->DecRef();
        pass->bindDepthStencilState(ds);
        ds->DecRef();
        pass->bindRasterizerState(rs);
        rs->DecRef();
        pass->setProgramID(ShaderType_VERTEXPROG, hDebugShaderResourceID_VertexPosCol);
        pass->setProgramID(ShaderType_FRAGMENTPROG, hDebugShaderResourceID_PixelPosCol);
        
        //ddrawmat->setResourceID(hResourceID::buildResourceID("?builtin.debug_pos_col_alpha_material"));
        ddrawmat->listenToResourceEvents(resmana);

        return ddrawmat;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hFont* HEART_API createDebugFont(hRenderer* rndr, hResourceManager* resmana, hFont* outfont, hTexture** outtex) {
        hcAssert(rndr && outfont && outtex);

        outfont->SetFontHeight((hUint32)g_debugfontHeight);
        outfont->SetFontWidth(0);
        outfont->SetPageCount(g_debugpageCount);
        outfont->SetPageResourceID(hStringID());
        outfont->SetMaterialResourceID(hStringID());
        outfont->SetFontCharacterLimit(g_debugglyphCount);

        for (hUint32 i = 0; i < g_debugglyphCount; ++i) {
            outfont->AddFontCharacter(g_debugglyphs+i);
        }
        outfont->SortCharacters();

        hMipDesc mipsdesc[] = {
            {(hUint32)low_res_fnt_width, (hUint32)low_res_fnt_height, (hByte*)low_res_fnt_data, (hUint)low_res_fnt_data_len},
        };
        rndr->createTexture(1, mipsdesc, eTextureFormat_R8_unorm, RESOURCEFLAG_DONTOWNCPUDATA, outtex);

#if 0
        resmana->insertResourceContainer(hStringID("?builtin/debug_font_surface"), *outtex, (*outtex)->getTypeName());
        resmana->insertResourceContainer(hStringID("?builtin/debug_font"), outfont, outfont->getTypeName());
#else
        hcPrintf("Stub "__FUNCTION__);
#endif

        return outfont;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    void HEART_API destroyDebugFont(hRenderer* rndr, hResourceManager* resmana, hFont* font, hTexture* tex) {
        hcAssert(rndr && font && tex);
        hFont* fontres=hResourceHandle("?builtin.debug_font").weakPtr<hFont>();
#if 0
        resmana->removeResource("?builtin.debug_font");
#else
        hcPrintf("Stub "__FUNCTION__);
#endif
        hDELETE(fontres);

        hTexture* texres=hResourceHandle("?builtin.debug_font_surface").weakPtr<hTexture>();
#if 0
        resmana->removeResource("?builtin.debug_font_surface");
#else
    hcPrintf("Stub "__FUNCTION__);
#endif
        tex->DecRef();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    void HEART_API submitDrawCalls(hRenderSubmissionCtx* ctx, hRendererCamera* camera, 
    const hDrawCall* dcs, hUint dcn, hUint32 clearflags) {
        HEART_PROFILE_FUNC();
        camera->UpdateParameters(ctx);
        ctx->setTargets(camera->getTargetCount(), camera->getTargets(), camera->getDepthTarget());
        ctx->SetViewport(camera->getTargetViewport());
        if ((clearflags&eClearTarget_Colour)==eClearTarget_Colour) {
            // Should clear flags be on the camera?
            for (hUint i=0; i<camera->getTargetCount(); ++i) {
                ctx->clearColour(camera->getRenderTarget(i), BLACK);
            }
        }
        if ((clearflags&eClearTarget_Depth)==eClearTarget_Depth) {
            ctx->clearDepth(camera->getDepthTarget(), 1.f);
        }

        const hMaterial* material = NULL;
        hMaterial* materialInst = NULL;
        hUint32 lastMatKey = 0;
        hUint32 pass = ~0U;
        hUint32 tmask = ~0U;
        for (hUint dc=0; dc < dcn; ++dc)
        {
            const hDrawCall* dcall = &dcs[dc];
            // For masks check hBuildRenderSortKey()
//             hUint32 nCam = (dcall->sortKey_&0xF000000000000000) >> 60;
//             hUint32 nPass = (dcall->sortKey_&0xF);
//             hUint32 matKey = (dcall->sortKey_&0x3FFFFF); // stored in lower 28 bits
// 
//             hBool newMaterial = matKey != lastMatKey;
//             lastMatKey = matKey;
//             //if (newMaterial){ //TODO flush correctly
//             ctx->setRenderStateBlock(dcall->blendState_);
//             ctx->setRenderStateBlock(dcall->depthState_);
//             ctx->setRenderStateBlock(dcall->rasterState_);
// 
//             ctx->SetRenderInputObject(dcall->progInput_);
//             ctx->SetInputStreams(&dcall->streams_);
//             if (dcall->instanceCount_) {
//                 ctx->DrawIndexedPrimitiveInstanced(dcall->instanceCount_, dcall->drawPrimCount_, 0);
//             }else{
//                 ctx->DrawIndexedPrimitive(dcall->drawPrimCount_, 0);
//             }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    static int drawCallCompare(const void* lhs, const void* rhs)
    {
#if 1
        if (((hDrawCall*)lhs)->sortKey_ < ((hDrawCall*)rhs)->sortKey_) {
            return -1;
        } else if ((((hDrawCall*)lhs)->sortKey_ > ((hDrawCall*)rhs)->sortKey_)) {
            return 1;
        } else {
            return 0;
        }
#else
        return (hInt32)((hInt64)((hDrawCall*)rhs)->sortKey_ - (hInt64)((hDrawCall*)lhs)->sortKey_);
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    void HEART_API sortDrawCalls(hDrawCall* dcs, hUint dcn) {
        HEART_PROFILE_FUNC();
        qsort(dcs, dcn, sizeof(hDrawCall), drawCallCompare);
    }

    HEART_DLLEXPORT
    void HEART_API setCameraParameters(hRenderSubmissionCtx* ctx, hRendererCamera* camera)
    {
        HEART_PROFILE_FUNC();
        camera->UpdateParameters(ctx);
        ctx->setTargets(camera->getTargetCount(), camera->getTargets(), camera->getDepthTarget());
        ctx->SetViewport(camera->getTargetViewport());
    }

}
}