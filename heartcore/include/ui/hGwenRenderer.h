/********************************************************************

	filename: 	hGwenRenderer.h	
	
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
#ifndef HGWENRENDERER_H__
#define HGWENRENDERER_H__

namespace Heart
{
    class HEARTCORE_SLIBEXPORT hGwenRenderer : public Gwen::Renderer::Base
    {
    public:
        hGwenRenderer() 
            : renderer_(NULL)
            , resourceManager_(NULL)
        {}
        ~hGwenRenderer() 
        {}

        void SetRenderCameraID(hByte layer) { rndCameraID_ = layer; }

        void Initialise(hRenderer* renderer, hResourceManager* resourceManager);
        void DestroyResources();
        void Begin();
        void End();
        void SetDrawColor( Gwen::Color color );
        void DrawLine( int x, int y, int a, int b );
        void DrawFilledRect( Gwen::Rect rect );
        void StartClip();
        void EndClip();

        //You can ignore these if you just want to use the simple skin (rectangle based) and don't want to draw images on the GUI
        void LoadTexture( Gwen::Texture* pTexture );
        void FreeTexture( Gwen::Texture* pTexture );
        void DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect pTargetRect, float u1=0.0f, float v1=0.0f, float u2=1.0f, float v2=1.0f );
        void DrawMissingImage( Gwen::Rect pTargetRect );

        //TODO: implement this...However this will be costly with DX11
        Gwen::Color PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color& col_default = Gwen::Color( 255, 255, 255, 255 ) );

        //You can also ignore these if you don't care about having text on your GUI. That would be silly though
        void LoadFont( Gwen::Font* pFont );
        void FreeFont( Gwen::Font* pFont );
        void RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text );
        Gwen::Point MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text );

    private:

        enum DrawMode
        {
            DrawMode_None,
            DrawMode_Lines,
            DrawMode_VertexColour,
            DrawMode_Textured,
        };

        struct ColourVtx
        {
            hFloat x,y,z;
            hColour colour;
        };

        struct ColourUVVtx
        {
            hFloat x,y,z;
            hColour colour;
            hFloat u,v;
        };

        void                    CreateRenderResources();
        void                    SwitchDrawMode(DrawMode newMode);
        void                    BeginDrawMode();
        void                    Flush();
        void                    BeginTextDraw(hFont* font, hUint32 nChars);
        void                    EndTextDraw(hUint32 nChars);

        hRenderer*                  renderer_;
        hDrawCallContext            dcCtx_;
        hResourceManager*           resourceManager_;
        hRendererCamera             renderCamera_;
        hMaterial*                  defaultMaterialResource_;
        hMaterialInstance*          defaultMaterial_;
        hMaterial*                  uvMaterialResource_;
        hMaterialInstance*          uvMaterial_;
        const hSamplerParameter*    uvSampler_;
        hByte*                      inlineVtxMem_;
        hUint16*                    inlinePrimCount_;
        hUint32*                    inlineVBSize_;
        hUint32                     currentStride_;
        hByte*                      inlineTxtVtxMem_;
        hByte*                      inlineTxtIdxMem_;
        hUint16*                    inlineTxtPC_;
        hUint32                     textVtxStride_;
        hFont*                      activeFont_;
        hUint32                     charactersLeft_;
        ScissorRect                 screenRect_;
        Gwen::Texture*              currentTexture_;

        hUint16*                    tmpIB;
        hByte*                      tmpVB;

        hUint32                 primCount_;
        hColour                 currentColour_;
        hUint32                 currectColourUint_;
        DrawMode                activeDrawMode_;

        hBool                   createdResources_;
        hByte                   rndCameraID_;
        hFloat                  depth_;
    };
}

#endif // HGWENRENDERER_H__