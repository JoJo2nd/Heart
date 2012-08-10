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
HEARTCORE_SLIBEXPORT
void* HEART_API GWENmalloc(size_t size)
{
    return hHeapMalloc(Heart::GetGlobalHeap()/*!heap*/, size);
}

HEARTCORE_SLIBEXPORT
void HEART_API GWENfree(void* ptr)
{
    hHeapFree(Heart::GetGlobalHeap()/*!heap*/, ptr);
}

namespace Heart
{

#define IDX_BUFFER_MAX_SIZE (1024)
#define VTX_BUFFER_MAX_SIZE (IDX_BUFFER_MAX_SIZE*4)
#define COLOUR_VTX_LAYOUT   (hrVF_XYZ | hrVF_COLOR)
#define UV_VTX_LAYOUT       (hrVF_XYZ | hrVF_COLOR | hrVF_1UV)

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
        currentTexture_ = NULL;
        activeDrawMode_ = DrawMode_None;

        if (!createdResources_)
            CreateRenderResources();

        hcAssert(defaultMaterial_);

        dcCtx_.Begin(renderer_);

        //ctx_->SetRendererCamera( &renderCamera_ );
        //ctx_->SetWorldMatrix( Heart::hMatrixFunc::identity() );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::End()
    {
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
        SwitchDrawMode(DrawMode_Lines);

        if ((primCount_+2)*3 > VTX_BUFFER_MAX_SIZE)
        {
            Flush();
            BeginDrawMode();
        }

        x += GetRenderOffset().x;
        y += GetRenderOffset().y;

        a += GetRenderOffset().x;
        b += GetRenderOffset().y;

        ColourVtx* vtx = (ColourVtx*)inlineVtxMem_;
        vtx += (primCount_*2);
        
        vtx->x = x;
        vtx->y = y;
        vtx->z = 1.f;
        vtx->colour = currentColour_;
        ++vtx;

        vtx->x = a;
        vtx->y = b;
        vtx->z = 1.f;
        vtx->colour = currentColour_;

        ++primCount_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::DrawFilledRect( Gwen::Rect rect )
    {
        SwitchDrawMode(DrawMode_VertexColour);

        if ((primCount_+2)*3 > VTX_BUFFER_MAX_SIZE)
        {
            Flush();
            BeginDrawMode();
        }

        rect.x += GetRenderOffset().x;
        rect.y += GetRenderOffset().y;

        ColourVtx* vtx = (ColourVtx*)inlineVtxMem_;
        vtx += (primCount_*3);

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
        ++primCount_;

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
        ++primCount_;

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::StartClip()
    {
        Flush();
        BeginDrawMode();

        ScissorRect rect;
        rect.left_      = ClipRegion().x;
        rect.top_       = ClipRegion().y;
        rect.right_     = ClipRegion().x+ClipRegion().w;
        rect.bottom_    = ClipRegion().y+ClipRegion().h;
        //TODO: support this
        //ctx_->SetScissorRect(rect);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::EndClip()
    {
        Flush();
        BeginDrawMode();

        //TODO: support this
        //ctx_->SetScissorRect(screenRect_);
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
        SwitchDrawMode(DrawMode_Textured);

        if ((primCount_+2)*3 > VTX_BUFFER_MAX_SIZE || texture != currentTexture_ )
        {
            Flush();
            BeginDrawMode();

            currentTexture_ = texture;
            uvMaterial_->SetSamplerParameter(uvSampler_, static_cast<hTexture*>(texture->data));
        }

        rect.x += GetRenderOffset().x;
        rect.y += GetRenderOffset().y;

        ColourUVVtx* vtx = (ColourUVVtx*)inlineVtxMem_;
        vtx += (primCount_*3);

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
        ++primCount_;

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
        ++primCount_;
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
        if (!pFont || !pFont->data)
            return;

        pos.x += GetRenderOffset().x;
        pos.y += GetRenderOffset().y;
        Gwen::String ctext = Gwen::Utility::UnicodeToString(text);

        hFont* font = static_cast<hFont*>(pFont->data);
        hFontStyle style = font->GetFontStyle();
        style.Alignment_ |= FONT_ALIGN_FLIP;
        style.Colour_ = currentColour_;

        BeginTextDraw(font, ctext.length());

        font->SetFontStyle(style);
        font->RenderStringSingleLine( inlineTxtIdxMem_, inlineTxtVtxMem_, hCPUVec2((hFloat)pos.x, (hFloat)pos.y), ctext.c_str());

        EndTextDraw(ctext.length());
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    Gwen::Point hGwenRenderer::MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text )
    {
        if (!pFont)
            return Gwen::Point();

        if (!pFont->data)
            LoadFont(pFont);

        if (!pFont->data)
            return Gwen::Point();

        hFont* font = static_cast<hFont*>(pFont->data);
        Gwen::String ctext = Gwen::Utility::UnicodeToString(text);
        hCPUVec2 v = font->CalcRenderSize(ctext.c_str());
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
        if (inlinePrimCount_ != NULL)
        {
            *inlinePrimCount_ = primCount_;
            inlinePrimCount_ = NULL;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::BeginDrawMode()
    {
        switch(activeDrawMode_)
        {
        case DrawMode_Lines:
        case DrawMode_VertexColour:
            dcCtx_.SubmitDrawCallInline(
                defaultMaterial_, 
                &inlineVtxMem_, 
                VTX_BUFFER_MAX_SIZE*renderer_->ComputeVertexLayoutStride(COLOUR_VTX_LAYOUT), 
                &inlinePrimCount_, hTrue );
            break;
        case DrawMode_Textured:
            dcCtx_.SubmitDrawCallInline(
                uvMaterial_, 
                &inlineVtxMem_, 
                VTX_BUFFER_MAX_SIZE*renderer_->ComputeVertexLayoutStride(UV_VTX_LAYOUT), 
                &inlinePrimCount_, hTrue );
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
        if (defaultMaterialResource_)
            defaultMaterialResource_->DestroyMaterialInstance(defaultMaterial_);
        if (uvMaterialResource_)
            uvMaterialResource_->DestroyMaterialInstance(uvMaterial_);
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
        defaultMaterial_ = defaultMaterialResource_->CreateMaterialInstance();

        uvMaterialResource_ = static_cast<hMaterial*>(resourceManager_->mtGetResource("CORE.UI_UV"));
        uvMaterial_ = uvMaterialResource_->CreateMaterialInstance();
        uvSampler_ = uvMaterial_->GetSamplerParameterByName("diffuseSampler");

        textVtxStride_ = renderer_->ComputeVertexLayoutStride(UV_VTX_LAYOUT);

        // 
        renderCamera_.Initialise( renderer_ );
        renderCamera_.SetOrthoParams( 0, 0, (hFloat)screenRect_.right_, (hFloat)screenRect_.bottom_, -0.0f, 2.0f );
        renderCamera_.SetViewMatrix( hMatrixFunc::identity() );
        renderCamera_.SetViewport( hViewport( 0, 0, screenRect_.right_, screenRect_.bottom_ ) );
        renderCamera_.SetTechniquePass(renderer_->GetMaterialManager()->GetRenderTechniqueInfo("main"));

        createdResources_ = hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::BeginTextDraw(hFont* font, hUint32 nChars)
    {
        if (font != activeFont_ || nChars > charactersLeft_)
        {
            activeFont_ = font;
            charactersLeft_ = hMax(256, nChars);
            dcCtx_.SubmitDrawCallInline(
                activeFont_->GetMaterialInstance(),
                &inlineTxtIdxMem_, charactersLeft_*6*sizeof(hUint16), 
                &inlineTxtVtxMem_, charactersLeft_*4*textVtxStride_, 
                &inlineTxtPC_, hTrue);
        }
        
        charactersLeft_ -= nChars;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hGwenRenderer::EndTextDraw( hUint32 nChars )
    {
        inlineTxtIdxMem_ += sizeof(hUint16)*charactersLeft_;
        inlineTxtVtxMem_ += textVtxStride_*charactersLeft_;
    }

}