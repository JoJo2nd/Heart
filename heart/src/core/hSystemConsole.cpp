/********************************************************************

    filename: 	hSystemConsole.cpp	
    
    Copyright (c) 22:1:2012 James Moran
    
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

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    class hConsoleUI : public hDebugMenuBase
    {
    public:
        //GWEN_CONTROL_CONSTRUCTOR(hConsoleUI)
        hConsoleUI(hSystemConsole* systemConsole) 
            : console_(systemConsole)
        {

        }

        ~hConsoleUI()
        {

        }

        void InitRenderResources(hRenderer* renderer, hResourceManager* resmanager)
        {
            hInputLayoutDesc layout[] = {
                {eIS_POSITION, 0, eIF_FLOAT3, 0, 0},
                {eIS_COLOUR,   0, eIF_FLOAT4, 0, 0},
                {eIS_TEXCOORD, 0, eIF_FLOAT2, 0, 0},
            };
            struct Vertex {
                hFloat x,y,z;
                hFloat r,g,b,a;
            };
            Vertex consolePlane[] = {
                {-1000.5f, 1000.f, 0.f, 0.f, 0.75f, 0.f, 0.2f }, 
                { 1000.5f, 1000.f, 0.f, 0.f, 0.75f, 0.f, 0.2f }, 
                {-1000.5f, 0.f, 0.f, 0.f, 0.75f, 0.f, 0.2f }, 

                {-1000.5f, 0.f, 0.f, 0.f, 0.75f, 0.f, 0.2f }, 
                { 1000.5f, 1000.f, 0.f, 0.f, 0.75f, 0.f, 0.2f }, 
                { 1000.5f, 0.f, 0.f, 0.f, 0.75f, 0.f, 0.2f }, 
            };

            renderer->CreateVertexBuffer(consolePlane, 6, layout, hStaticArraySize(layout)-1, 0, GetDebugHeap(), &backdropPlane_);
            backdropMat_ = static_cast< hMaterial* >(resmanager->mtGetResource("CORE.DEBUG_MAT"));

            renderer->CreateVertexBuffer(textBuffer_, INPUT_BUFFER_LEN*6, layout, hStaticArraySize(layout), RESOURCEFLAG_DYNAMIC, GetDebugHeap(), &textBuffer_);
            renderer->CreateVertexBuffer(logBuffer_, hSystemConsole::MAX_CONSOLE_LOG_SIZE*6, layout, hStaticArraySize(layout), RESOURCEFLAG_DYNAMIC, GetDebugHeap(), &logBuffer_);
            consoleFont_ = static_cast< hFont* >(resmanager->mtGetResource("CORE.CONSOLE"));
            fontMat_ = static_cast< hMaterial* >(resmanager->mtGetResource("CORE.FONT_MAT"));

            debugTechMask_ = renderer->GetMaterialManager()->GetRenderTechniqueInfo("main")->mask_;
        }
        void PreRenderUpdate() {}
        void Render(hRenderSubmissionCtx* ctx, hdParameterConstantBlock* instanceCB, const hDebugRenderParams& params) 
        {
            hFloat intprt = 0.f;
            hInstanceConstants* inst;
            hConstBlockMapInfo map;
            hVertexBufferMapInfo vbmap;
            hFloat fontScale = 1.f;//+(.2f*sin(hClock::elapsed()*.24f));
            hCPUVec2 bottomleft(-(params.rtWidth_/2.f), -(params.rtHeight_/2.f));
            hCPUVec2 bottomright((params.rtWidth_/2.f), -(params.rtHeight_/2.f));
            hCPUVec2 topleft(-(params.rtWidth_/2.f), 1000.f);
            hUint32 prims = 0;
            hFontFormatting formatter;
            formatter.setScale(fontScale);
            formatter.setFont(consoleFont_);
            formatter.setColour(WHITE);

            if (GetVisible()) {

                ctx->Map(instanceCB, &map);
                inst = (hInstanceConstants*)map.ptr;
                inst->world_ = hMatrixFunc::identity();
                ctx->Unmap(&map);

                hMaterialTechnique* tech = backdropMat_->GetTechniqueByMask(debugTechMask_);
                if (!tech) return;
                for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
                    hMaterialTechniquePass* passptr = tech->GetPass(pass);
                    ctx->SetVertexStream(0, backdropPlane_, backdropPlane_->GetStride());
                    ctx->SetMaterialPass(passptr);
                    ctx->SetPrimitiveType(PRIMITIVETYPE_TRILIST);
                    ctx->DrawPrimitive(2, 0);
                }

                {
                    hChar* teststr = "The Red Fox jumped over the Lazy Dog 1234567890!";
                    ctx->Map(textBuffer_, &vbmap);
                    formatter.setOutputBuffer(vbmap.ptr_, INPUT_BUFFER_LEN*6*sizeof(hFontVex));
                    formatter.setInputStringBuffer(teststr, hStrLen(teststr));
                    formatter.setFormatExtents(FLT_MAX, 0);
                    formatter.setAlignment(hFONT_ALIGN_LEFT|hFONT_ALIGN_TOP);
                    formatter.formatText();
                    formatter.writeTextToBuffer(bottomleft);
                    prims = formatter.getPrimitiveCount();
                    ctx->Unmap(&vbmap);
                }

                tech = fontMat_->GetTechniqueByMask(debugTechMask_);
                if (!tech) return;
                for (hUint32 pass = 0, passcount = tech->GetPassCount() && prims; pass < passcount; ++pass ) {
                    hMaterialTechniquePass* passptr = tech->GetPass(pass);
                    ctx->SetVertexStream(0, textBuffer_, textBuffer_->GetStride());
                    ctx->SetMaterialPass(passptr);
                    ctx->SetPrimitiveType(PRIMITIVETYPE_TRILIST);
                    ctx->DrawPrimitive(prims, 0);
                }

                {
                    ctx->Map(logBuffer_, &vbmap);
                    formatter.setOutputBuffer(vbmap.ptr_, hSystemConsole::MAX_CONSOLE_LOG_SIZE*6*sizeof(hFontVex));
                    formatter.setInputStringBuffer(log_, logSize_);
                    formatter.setFormatExtents(FLT_MAX, FLT_MAX);
                    formatter.setAlignment(hFONT_ALIGN_LEFT|hFONT_ALIGN_BOTTOM);
                    formatter.formatText();
                    formatter.writeTextToBuffer(bottomleft);
                    prims = formatter.getPrimitiveCount();
                    ctx->Unmap(&vbmap);
                }

                tech = fontMat_->GetTechniqueByMask(debugTechMask_);
                if (!tech) return;
                for (hUint32 pass = 0, passcount = tech->GetPassCount() && prims; pass < passcount; ++pass ) {
                    hMaterialTechniquePass* passptr = tech->GetPass(pass);
                    ctx->SetVertexStream(0, logBuffer_, logBuffer_->GetStride());
                    ctx->SetMaterialPass(passptr);
                    ctx->SetPrimitiveType(PRIMITIVETYPE_TRILIST);
                    ctx->DrawPrimitive(prims, 0);
                }
            }
        }
        void EndFrameUpdate() {}
        void updateConsoleLogString(hSystemConsole::hConsoleLogType& log, hChar* inputStr) 
        { 
            log.copyToBuffer(log_, &logSize_); 
            hStrCopy(inputBuffer_, INPUT_BUFFER_LEN, inputStr);
        }

    private:

        static const hUint32 INPUT_BUFFER_LEN = hSystemConsole::INPUT_BUFFER_LEN;

        hUint32                  debugTechMask_;
        hVertexBuffer*           textBuffer_;
        hVertexBuffer*           logBuffer_;
        hVertexBuffer*           backdropPlane_;
        hMaterial*               backdropMat_;
        hFont*                   consoleFont_;
        hMaterial*               fontMat_;

        hSystemConsole* console_;
        hUint32         logSize_;
        hChar           log_[hSystemConsole::MAX_CONSOLE_LOG_SIZE];
        hChar           inputBuffer_[INPUT_BUFFER_LEN];
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hResourceID   hSystemConsole::FONT_RESOURCE_NAME = hResourceManager::BuildResourceID( "CORE.CONSOLE" );
    const hResourceID   hSystemConsole::CONSOLE_MATERIAL_NAME = hResourceManager::BuildResourceID( "CORE.FONT_MAT" );
    hMutex				hSystemConsole::messagesMutex_;
    hSystemConsole::hConsoleLogType	hSystemConsole::messageBuffer_;
    hUint32				hSystemConsole::msgBufferLen_ = 0;
    hBool				hSystemConsole::alive_ = hTrue;


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::Initialise( hControllerManager* pControllerManager,
                                     hLuaStateManager* lua,
                                     hResourceManager* pResourceManager,
                                     hRenderer* renderer,
                                     hGwenRenderer* uiRenderer)
    {
        controllerManager_ = pControllerManager;
        resourceManager_ = pResourceManager;
        renderer_ = renderer;
        vm_ = lua;
        keyboard_ = controllerManager_->GetSystemKeyboard();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::Destroy()
    {
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::Update()
    {
        HEART_PROFILE_FUNC();
        if ( !loaded_ )
        {
            if ( hDebugMenuManager::GetInstance()->Ready() )
            {
                //////////////////////////////////////////////////////////////////////////
                // Create Menu for displaying the console ////////////////////////////////
                //////////////////////////////////////////////////////////////////////////
                loaded_ = hTrue;

                consoleWindow_ = hNEW(GetDebugHeap(), hConsoleUI)(this);
                consoleWindow_->InitRenderResources(renderer_, resourceManager_);
                hDebugMenuManager::GetInstance()->RegisterMenu("console",consoleWindow_);
            }
        }
        else
        {
            UpdateConsole();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::ClearConsoleBuffer()
    {
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::ExecuteBuffer(const hChar* input)
    {
        //add to the log of inputted commands
        PrintConsoleMessage(input);

        //Run the command
        vm_->ExecuteBuffer(input, hStrLen(input));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::UpdateConsole()
    {
        if (consoleWindow_)
        {
            if (keyboard_->GetButton(VK_SHIFT).buttonVal_ && keyboard_->GetButton(VK_BACK).raisingEdge_)
            {
                visible_ = !visible_;
                hDebugMenuManager::GetInstance()->SetMenuVisiablity("console", visible_);
            }

            hMutexAutoScope mas(&messagesMutex_);
            consoleWindow_->updateConsoleLogString(messageBuffer_, inputBuffer_);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::PrintConsoleMessage( const hChar* string )
    {
        hMutexAutoScope mas(&messagesMutex_);

        hUint32 len = hStrLen(string);
        for (hUint32 i = 0; i < len; ++i) {
            messageBuffer_.pushChar(string[i]);
        }
        
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::ClearLog()
    {
    }
}