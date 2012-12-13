/********************************************************************

	filename: 	hGwenRenderer.cpp	
	
	Copyright (c) 5:5:2012 James Moran
	
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

//Temp placement
HEART_DLLEXPORT
void* HEART_API GWENmalloc(size_t size)
{
    return hHeapMalloc(Heart::GetDebugHeap()/*!heap*/, size);
}

HEART_DLLEXPORT
void HEART_API GWENfree(void* ptr)
{
    hHeapFree(Heart::GetDebugHeap()/*!heap*/, ptr);
}

namespace Heart
{

#define IDX_BUFFER_MAX_SIZE (1024)
#define VTX_BUFFER_MAX_SIZE (IDX_BUFFER_MAX_SIZE*4)

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::Initialise( hRenderer* renderer, hResourceManager* resourceManager )
    {
        renderer_ = renderer;
        resourceManager_ = resourceManager;
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::Begin()
    {
        HEART_PROFILE_FUNC();
        currentTexture_ = NULL;
        activeDrawMode_ = DrawMode_None;
        depth_ = 0.f;

        if (!createdResources_)
            CreateRenderResources();

        hcAssert(defaultMaterial_);

        dcCtx_.SetCameraID(rndCameraID_);
        dcCtx_.SetRenderLayer(0);
        dcCtx_.SetScissor(0, 0, renderer_->GetWidth(), renderer_->GetHeight());
        dcCtx_.Begin(renderer_);
        

        vcVBPtr_ = (hByte*)dcCtx_.Map(vtxColourVB_, s_maxDrawPrims*vcStride_);
        vcIBPtr_ = (hUint16*)dcCtx_.Map(vtxColourIB_, s_maxDrawPrims*sizeof(hUint16));
        vcStart_ = 0;
        tVBPtr_ = (hByte*)dcCtx_.Map(texturedVB_, s_maxDrawPrims*textVtxStride_);
        tIBPtr_ = (hUint16*)dcCtx_.Map(texturedIB_, s_maxDrawPrims*sizeof(hUint16));
        tVBStart_ = 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::End()
    {
        HEART_PROFILE_FUNC();
        Flush();
        dcCtx_.End();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::SetDrawColor( Gwen::Color color )
    {
        currentColour_ = hColour(color.r, color.g, color.b, color.a);
        currectColourUint_ = (hUint32)currentColour_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::DrawLine( int x, int y, int a, int b )
    {
        HEART_PROFILE_FUNC();
        SwitchDrawMode(DrawMode_Lines);

        x += GetRenderOffset().x;
        y += GetRenderOffset().y;

        a += GetRenderOffset().x;
        b += GetRenderOffset().y;

        ColourVtx* vtx = (ColourVtx*)inlineVtxMem_;
        inlineVtxMem_ += (vtxCount_*2);
        
        vtx->x = x;
        vtx->y = y;
        vtx->z = 1.f;
        vtx->colour = currentColour_;
        ++vtx;

        vtx->x = a;
        vtx->y = b;
        vtx->z = 1.f;
        vtx->colour = currentColour_;

        vtxCount_+=2;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::DrawFilledRect( Gwen::Rect rect )
    {
        HEART_PROFILE_FUNC();
        SwitchDrawMode(DrawMode_VertexColour);

        rect.x += GetRenderOffset().x;
        rect.y += GetRenderOffset().y;

        ColourVtx* vtx = (ColourVtx*)inlineVtxMem_;

        vtx->x = rect.x;
        vtx->y = rect.y;
        vtx->z = 1.f;
        vtx->colour = currentColour_;
        ++vtx;

        vtx->x = rect.x+rect.w;
        vtx->y = rect.y;
        vtx->z = 1.f;
        vtx->colour = currentColour_;
        ++vtx;

        vtx->x = rect.x;
        vtx->y = rect.y+rect.h;
        vtx->z = 1.f;
        vtx->colour = currentColour_;
        ++vtx;
        vtxCount_+=3;

        vtx->x = rect.x;
        vtx->y = rect.y+rect.h;
        vtx->z = 1.f;
        vtx->colour = currentColour_;
        ++vtx;

        vtx->x = rect.x+rect.w;
        vtx->y = rect.y;
        vtx->z = 1.f;
        vtx->colour = currentColour_;
        ++vtx;

        vtx->x = rect.x+rect.w;
        vtx->y = rect.y+rect.h;
        vtx->z = 1.f;
        vtx->colour = currentColour_;
        ++vtx;
        vtxCount_+=3;

        inlineVtxMem_ = (hByte*)vtx;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::StartClip()
    {
        Flush();
        dcCtx_.SetScissor(ClipRegion().x-1, ClipRegion().y-1, ClipRegion().x+ClipRegion().w-1, ClipRegion().y+ClipRegion().h-1);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::EndClip()
    {
        Flush();
        dcCtx_.SetScissor(ClipRegion().x, ClipRegion().y, ClipRegion().x+ClipRegion().w, ClipRegion().y+ClipRegion().h);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::LoadTexture( Gwen::Texture* pTexture )
    {
        if (!pTexture->data)
        {
            hTexture* text = static_cast<hTexture*>(resourceManager_->mtGetResource(pTexture->name.Get().c_str()));

            if (text)
            {
                pTexture->width = text->Width();
                pTexture->height = text->Height();
                pTexture->data = text;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::FreeTexture( Gwen::Texture* /*pTexture*/ )
    {
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::DrawTexturedRect( Gwen::Texture* texture, Gwen::Rect rect, float u1/*=0.0f*/, float v1/*=0.0f*/, float u2/*=1.0f*/, float v2/*=1.0f */ )
    {
        HEART_PROFILE_FUNC();
        SwitchDrawMode(DrawMode_Textured);

        if (texture != currentTexture_)
        {
            Flush();
            BeginDrawMode();
            currentTexture_ = texture;
            //uvMaterial_->SetSamplerParameter(uvSampler_, static_cast<hTexture*>(texture->data));
        }

        rect.x += GetRenderOffset().x;
        rect.y += GetRenderOffset().y;

        ColourUVVtx* vtx = (ColourUVVtx*)inlineVtxMem_;

        vtx->x = -0.5f + rect.x;
        vtx->y = -0.5f + rect.y;
        vtx->z = 1.f;
        vtx->colour = currentColour_;
        vtx->u = u1;
        vtx->v = v1;
        ++vtx;

        vtx->x = -0.5f + (rect.x+rect.w);
        vtx->y = -0.5f + (rect.y);
        vtx->z = 1.f;
        vtx->colour = currentColour_;
        vtx->u = u2;
        vtx->v = v1;
        ++vtx;

        vtx->x = -0.5f + (rect.x);
        vtx->y = -0.5f + (rect.y+rect.h);
        vtx->z = 1.f;
        vtx->colour = currentColour_;
        vtx->u = u1;
        vtx->v = v2;
        ++vtx;
        vtxCount_+=3;

        vtx->x = -0.5f + (rect.x);
        vtx->y = -0.5f + (rect.y+rect.h);
        vtx->z = 1.f;
        vtx->colour = currentColour_;
        vtx->u = u1;
        vtx->v = v2;
        ++vtx;

        vtx->x = -0.5f + (rect.x+rect.w);
        vtx->y = -0.5f + rect.y;
        vtx->z = 1.f;
        vtx->colour = currentColour_;
        vtx->u = u2;
        vtx->v = v1;
        ++vtx;

        vtx->x = -0.5f + (rect.x+rect.w);
        vtx->y = -0.5f + (rect.y+rect.h);
        vtx->z = 1.f;
        vtx->colour = currentColour_;
        vtx->u = u2;
        vtx->v = v2;
        ++vtx;
        vtxCount_+=3;

        inlineVtxMem_ = (hByte*)vtx;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::DrawMissingImage( Gwen::Rect pTargetRect )
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::LoadFont( Gwen::Font* pFont )
    {
        Gwen::String uri = Gwen::Utility::UnicodeToString(pFont->facename.c_str());
        pFont->data = resourceManager_->mtGetResource(uri.c_str());
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::FreeFont( Gwen::Font* pFont )
    {
        pFont->data = NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text )
    {
        HEART_PROFILE_FUNC();
        /*if (!pFont || !pFont->data)
            return;

        Flush();
        SwitchDrawMode(DrawMode_Text);

        pos.x += GetRenderOffset().x;
        pos.y += GetRenderOffset().y;
        Gwen::Utility::UnicodeToString(text, ctext_);

        hFont* font = static_cast<hFont*>(pFont->data);
        hFontStyle style = font->GetFontStyle();
        style.Alignment_ |= 0;
        style.Colour_ = currentColour_;

        if (activeFont_ != font)
        {
            Flush();
        }

        hUint32 nChars = ctext_.length();
        activeFont_ = font;

        font->SetFontStyle(style);
        font->RenderStringSingleLine( inlineVtxMem_, hCPUVec2((hFloat)pos.x, (hFloat)pos.y), ctext_.c_str());

        inlineVtxMem_ += nChars*6*textVtxStride_;
        vtxCount_ += nChars*6;*/
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    Gwen::Point hGwenRenderer::MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text )
    {
        HEART_PROFILE_FUNC();
        if (!pFont)
            return Gwen::Point();

        if (!pFont->data)
            LoadFont(pFont);

        if (!pFont->data)
            return Gwen::Point();

        hFont* font = static_cast<hFont*>(pFont->data);
        Gwen::Utility::UnicodeToString(text, ctext_);
        hUTF8Iterator itr(ctext_.c_str());
        hCPUVec2 v = font->CalcRenderSize(&itr);
        return Gwen::Point((hInt32)v.x,(hInt32)v.y);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::SwitchDrawMode( DrawMode newMode )
    {
        if (activeDrawMode_ == newMode)
            return;

        Flush();
        activeDrawMode_ = newMode;
        BeginDrawMode();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::Flush()
    {
        HEART_PROFILE_FUNC();
        if (vtxCount_ != 0)
        {
            switch(activeDrawMode_)
            {
            case DrawMode_Lines:
            case DrawMode_VertexColour:
                //dcCtx_.SubmitDrawCall(defaultMaterial_, NULL, vtxColourVB_, vtxCount_/3, hTrue, depth_, vcStart_);

                vcVBPtr_ = inlineVtxMem_;
                vcStart_ += vtxCount_;
                //Need assert to check buffer overrun
                hcAssertMsg(vcStart_ < s_maxDrawPrims, "Buffer Overrun");
                break;
            case DrawMode_Textured:
                //dcCtx_.SubmitDrawCall(uvMaterial_, NULL, texturedVB_, vtxCount_/3, hTrue, depth_, tVBStart_ );

                tVBPtr_ = inlineVtxMem_;
                tVBStart_ += vtxCount_;
                //Need assert to check buffer overrun
                hcAssertMsg(tVBStart_ < s_maxDrawPrims, "Buffer Overrun");
                break;
            case DrawMode_Text:
                //dcCtx_.SubmitDrawCall(activeFont_->GetMaterialInstance(), NULL, texturedVB_, vtxCount_/3, hTrue, depth_, tVBStart_ );

                tVBPtr_ = inlineVtxMem_;;
                tVBStart_ += vtxCount_;
                //Need assert to check buffer overrun
                hcAssertMsg(tVBStart_ < s_maxDrawPrims, "Buffer Overrun");
                break;
            default:
                break;
            }

            vtxCount_ = 0;
            depth_ += 1.f;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::BeginDrawMode()
    {
        HEART_PROFILE_FUNC();
        switch(activeDrawMode_)
        {
        case DrawMode_Lines:
        case DrawMode_VertexColour:
            currentStride_ = vcStride_;//renderer_->ComputeVertexLayoutStride(COLOUR_VTX_LAYOUT);
            inlineVtxMem_ = vcVBPtr_;
            break;
        case DrawMode_Textured:
        case DrawMode_Text:
            currentStride_ = textVtxStride_;//renderer_->ComputeVertexLayoutStride(UV_VTX_LAYOUT);
            inlineVtxMem_ = tVBPtr_;
            break;
        default:
            break;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::DestroyResources()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    Gwen::Color hGwenRenderer::PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color& col_default /*= Gwen::Color( 255, 255, 255, 255 ) */ )
    {
        if (!pTexture->data)
            return col_default;

        hTexture* text = static_cast<hTexture*>(pTexture->data);
        hColour c = text->ReadPixel(x,y);
        return Gwen::Color((hByte)(c.r_*255.f),(hByte)(c.g_*255.f),(hByte)(c.b_*255.f),(hByte)(c.a_*255.f));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::CreateRenderResources()
    {
        hcAssert(createdResources_ == hFalse);

        screenRect_.left_   = 0;
        screenRect_.top_    = 0;
        screenRect_.right_  = renderer_->GetWidth();
        screenRect_.bottom_ = renderer_->GetHeight();

        defaultMaterialResource_ = static_cast<hMaterial*>(resourceManager_->mtGetResource("CORE.UI_COLOUR"));
        if (!defaultMaterialResource_)
            return;
        //defaultMaterial_ = defaultMaterialResource_->CreateMaterialInstance();

        uvMaterialResource_ = static_cast<hMaterial*>(resourceManager_->mtGetResource("CORE.UI_UV"));
        if (!uvMaterialResource_)
            return;
        //uvMaterial_ = uvMaterialResource_->CreateMaterialInstance();
        //uvSampler_ = uvMaterial_->GetSamplerParameterByName("diffuseSampler");
        hcAssert(uvSampler_);

        hInputLayoutDesc vtxlayout[] = {
            {eIS_POSITION, 0, eIF_FLOAT3, 0, 0},
            {eIS_COLOUR,   0, eIF_FLOAT4, 0, 0},
            {eIS_TEXCOORD, 0, eIF_FLOAT2, 0, 0},
        };

        vcStride_      = renderer_->ComputeVertexLayoutStride(vtxlayout, 2);
        textVtxStride_ = renderer_->ComputeVertexLayoutStride(vtxlayout, 3);

        renderer_->CreateVertexBuffer(NULL, s_maxDrawPrims, vtxlayout, 2, RESOURCEFLAG_DYNAMIC, GetDebugHeap(), &vtxColourVB_);
        renderer_->CreateVertexBuffer(NULL, s_maxDrawPrims, vtxlayout, 3, RESOURCEFLAG_DYNAMIC, GetDebugHeap(), &texturedVB_);
        renderer_->CreateIndexBuffer(NULL, s_maxDrawPrims, RESOURCEFLAG_DYNAMIC, PRIMITIVETYPE_TRILIST, &vtxColourIB_);
        renderer_->CreateIndexBuffer(NULL, s_maxDrawPrims, RESOURCEFLAG_DYNAMIC, PRIMITIVETYPE_TRILIST, &texturedIB_);

        createdResources_ = hTrue;
    }

}