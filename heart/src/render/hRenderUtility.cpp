/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "render/hRenderUtility.h"
#include "render/hFont.h"
#include "base/hRendererConstants.h"
#include "render/hIndexBuffer.h"
#include "render/hVertexBuffer.h"
#include "math/hMathUtil.h"
#include "hRenderer.h"
#include "hMaterial.h"
#include "hTexture.h"
#include "base/hProfiler.h"

extern const float g_debugfontHeight;
extern const unsigned int g_debugpageCount;
extern const unsigned int g_debugglyphCount;
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
        pOutSamples[0] = hVec2(0.f, 0.f, 0.f);

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

            hVec2 delta = hVec2(dx, dy, 0.f) * sampleOffset;

            // Store texture coordinate offsets for the positive and negative taps.
            pOutSamples[i * 2 + 1] = delta;
            pOutSamples[i * 2 + 2] = -delta;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    void HEART_API buildSphereMesh(hUint16 segments, hUint16 rings, hFloat radius, 
    hIndexBuffer** outIdxBuf, hVertexBuffer** outVtxBuf)
    {
#if 0
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
                hVec3 vNormal = normalize( v );//needs flipping, but shouldn't

                // Add one vertex to the strip which makes up the sphere
                *vtx++ = x0;
                *vtx++ = y0;
                *vtx++ = z0;

				//Normal
	 			*vtx++ = vNormal.getX();
	 			*vtx++ = vNormal.getY();
	 			*vtx++ = vNormal.getZ();
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
#else
        hStub();
#endif
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
#if 0
        hUint16 quadIdx[] =
        {
            0,2,1,2,3,1
        };
        hFloat wo2 = width / 2.0f;
        hFloat ho2 = height / 2.0f;
        struct Vertex
        {
            hFloat pos[3];
            hFloat uv[2];
        } init_vtx [] = {
            {{-wo2,  ho2, 0.25f}, {0.f, 0.f}},
            {{ wo2,  ho2, 0.25f}, {1.f, 0.f}},
            {{-wo2, -ho2, 0.25f}, {0.f, 1.f}},
            {{ wo2, -ho2, 0.25f}, {1.f, 1.f}},
        };

        hIndexBufferMapInfo ibMapInfo;
        ctx->Map( idxBuf, &ibMapInfo );

        hMemCpy( ibMapInfo.ptr_, quadIdx, sizeof(quadIdx) );

        ctx->Unmap( &ibMapInfo );

        hVertexBufferMapInfo vbMapInfo;
        ctx->Map( vtxBuf, &vbMapInfo );
        hMemCpy(vbMapInfo.ptr_, init_vtx, sizeof(init_vtx));
        ctx->Unmap( &vbMapInfo );
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

     
    void HEART_API buildTessellatedQuadMesh(hFloat width, hFloat height, hUint wdivs, hUint hdivs, 
    hIndexBuffer** outIdxBuf, hVertexBuffer** outVtxBuf) {
#if 0
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
#else
        hStub();
#endif
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

     
    void HEART_API buildConeMesh(hUint16 segments, hFloat radius, hFloat depth, hVertexBuffer** outVtxBuf) {
#if 0
        hcAssert(outVtxBuf && rndr);
        hInputLayoutDesc desc[] = {
            hInputLayoutDesc("POSITION", 0, eIF_FLOAT3, 0, 0),
            hInputLayoutDesc("NORMAL", 0, eIF_FLOAT3, 0, 0),
        };
        struct Vertex
        {
            hFloat pos[3];
            hFloat normal[3];
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
            hVec3 n1 = cross(v3-v1, v2-v1);
            hVec3 n2 = cross(v1-v2, v3-v2);
            hVec3 n3 = cross(v2-v3, v1-v3);
            for (hUint i=0; i<3; ++i) {
                vtx[0].pos[i] = v1[i];
                vtx[1].pos[i] = v2[i];
                vtx[2].pos[i] = v3[i];
                vtx[0].normal[i]=n1[i];
                vtx[1].normal[i]=n2[i];
                vtx[2].normal[i]=n3[i];
            }
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
            hVec3 n1 = cross(v3-v1, v2-v1);
            hVec3 n2 = cross(v1-v2, v3-v2);
            hVec3 n3 = cross(v2-v3, v1-v3);
            for (hUint i=0; i<3; ++i) {
                vtx[0].pos[i] = v1[i];
                vtx[1].pos[i] = v2[i];
                vtx[2].pos[i] = v3[i];
                vtx[0].normal[i]=n1[i];
                vtx[1].normal[i]=n2[i];
                vtx[2].normal[i]=n3[i];
            }
//             vtx[0].pos = v1;
//             vtx[1].pos = v2;
//             vtx[2].pos = v3;
//             vtx[0].normal=hVec3Func::cross(v3-v1, v2-v1);
//             vtx[1].normal=hVec3Func::cross(v1-v2, v3-v2);
//             vtx[2].normal=hVec3Func::cross(v2-v3, v1-v3);
            vtx+=3;
        }

        rndr->createVertexBuffer(verts, vtxCnt, desc, (hUint32)hStaticArraySize(desc), 0, outVtxBuf);
#else
        hStub();
#endif
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

#define PRIM_COUNT (36)
    hVertexBuffer* buildDebugCubeMesh(hVertexBuffer** retVB) {
#if 0
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
#else
        hStub();
        return nullptr;
#endif
    }
#undef PRIM_COUNT

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hMaterial* HEART_API buildDebugFontMaterial(hMaterial* ddrawmat) {
#if 0
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
        ddrawmat->listenToResourceEvents();

        return ddrawmat;
#else
        hStub();
        return nullptr;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hMaterial* HEART_API buildDebugPosColUVMaterial(hMaterial* ddrawmat) {
#if 0
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
        ddrawmat->listenToResourceEvents();

        return ddrawmat;
#else
        hStub();
        return nullptr;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hMaterial* HEART_API buildDebugPosColMaterial(hMaterial* ddrawmat) {
#if 0
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
        ddrawmat->listenToResourceEvents();

        return ddrawmat;
#else
        hStub();
        return nullptr;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hMaterial* HEART_API buildDebugPosColUVAlphaMaterial(hMaterial* ddrawmat) {
#if 0
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
        ddrawmat->listenToResourceEvents();

        return ddrawmat;
#else
        hStub();
        return nullptr;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hMaterial* HEART_API buildDebugPosColAlphaMaterial(hMaterial* ddrawmat) {
#if 0
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
        ddrawmat->listenToResourceEvents();

        return ddrawmat;
#else
        hStub();
        return nullptr;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hFont* HEART_API createDebugFont(hFont* outfont, hTexture** outtex) {
#if 0
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

        resmana->insertResourceContainer(hStringID("?builtin/debug_font_surface"), *outtex, (*outtex)->getTypeName());
        resmana->insertResourceContainer(hStringID("?builtin/debug_font"), outfont, outfont->getTypeName());

        return outfont;
#else
        hStub();
        return nullptr;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    void HEART_API destroyDebugFont(hFont* font, hTexture* tex) {
#if 0
        hcAssert(rndr && font && tex);
        hFont* fontres=hResourceHandle("?builtin.debug_font").weakPtr<hFont>();
        resmana->removeResource("?builtin.debug_font");
        delete fontres;

        hTexture* texres=hResourceHandle("?builtin.debug_font_surface").weakPtr<hTexture>();
        resmana->removeResource("?builtin.debug_font_surface");
        tex->DecRef();
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    void HEART_API submitDrawCalls(hRenderSubmissionCtx* ctx, hRendererCamera* camera, 
    const hDrawCall* dcs, hUint dcn, hUint32 clearflags) {
#if 0
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
#else
        hStub();
#endif        
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    static int drawCallCompare(const void* lhs, const void* rhs)
    {
#if 0
        if (((hDrawCall*)lhs)->sortKey_ < ((hDrawCall*)rhs)->sortKey_) {
            return -1;
        } else if ((((hDrawCall*)lhs)->sortKey_ > ((hDrawCall*)rhs)->sortKey_)) {
            return 1;
        } else {
            return 0;
        }
#else
        hStub();
        return 0;//return (hInt32)((hInt64)((hDrawCall*)rhs)->sortKey_ - (hInt64)((hDrawCall*)lhs)->sortKey_);
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    void HEART_API sortDrawCalls(hDrawCall* dcs, hUint dcn) {
#if 0
        HEART_PROFILE_FUNC();
        qsort(dcs, dcn, sizeof(hDrawCall), drawCallCompare);
#else
        hStub();
#endif
    }

    
    void HEART_API setCameraParameters(hRenderSubmissionCtx* ctx, hRendererCamera* camera) {
#if 0
        HEART_PROFILE_FUNC();
        camera->UpdateParameters(ctx);
        ctx->setTargets(camera->getTargetCount(), camera->getTargets(), camera->getDepthTarget());
        ctx->SetViewport(camera->getTargetViewport());
#else
        hStub();
#endif
    }

}
}