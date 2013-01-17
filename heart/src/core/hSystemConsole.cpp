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

    hFloat              hSystemConsole::s_fontSize = 1.f;
    hConsoleOutputProc  hSystemConsole::s_consoleOutputCallback = NULL;
    void*               hSystemConsole::s_consoleOutputUser = NULL;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    class hConsoleUI : public hDebugMenuBase
    {
    public:
        //GWEN_CONTROL_CONSTRUCTOR(hConsoleUI)
        hConsoleUI(hSystemConsole* systemConsole) 
            : console_(systemConsole)
            , debugTechMask_(0)
            , textBuffer_(NULL)
            , logBuffer_(NULL)
            , backdropPlane_(NULL)
            , backdropMat_(NULL)
            , backdropCB_(NULL)
            , consoleFont_(NULL)
            , fontMat_(NULL)
            , windowOffset_(1.f)
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
                { .5f, .5f, 0.f, 0.f, 0.75f, 0.f, 0.2f }, 
                {-.5f, .5f, 0.f, 0.f, 0.75f, 0.f, 0.2f }, 
                {-.5f,-.5f, 0.f, 0.f, 0.75f, 0.f, 0.2f }, 

                {-.5f,-.5f, 0.f, 0.f, 0.75f, 0.f, 0.2f }, 
                { .5f, .5f, 0.f, 0.f, 0.75f, 0.f, 0.2f }, 
                { .5f,-.5f, 0.f, 0.f, 0.75f, 0.f, 0.2f }, 
            };

            renderer->CreateVertexBuffer(consolePlane, 6, layout, hStaticArraySize(layout)-1, 0, GetDebugHeap(), &backdropPlane_);
            hMaterial* backdropMat = static_cast< hMaterial* >(resmanager->mtGetResource("CORE.CONSOLE_MAT"));
            backdropMat_ = backdropMat->createMaterialInstance();
            backdropCB_ = backdropMat_->GetParameterConstBlock(hCRC32::StringCRC("ConsoleConstants"));

            renderer->CreateVertexBuffer(textBuffer_, INPUT_BUFFER_LEN*6, layout, hStaticArraySize(layout), RESOURCEFLAG_DYNAMIC, GetDebugHeap(), &textBuffer_);
            renderer->CreateVertexBuffer(logBuffer_, hSystemConsole::MAX_CONSOLE_LOG_SIZE*6, layout, hStaticArraySize(layout), RESOURCEFLAG_DYNAMIC, GetDebugHeap(), &logBuffer_);
            consoleFont_ = static_cast< hFont* >(resmanager->mtGetResource("CORE.CONSOLE"));
            hMaterial* fontMat = static_cast< hMaterial* >(resmanager->mtGetResource("CORE.FONT_MAT"));
            fontMat_ = fontMat->createMaterialInstance();
            fontCB_ = fontMat_->GetParameterConstBlock(hCRC32::StringCRC("FontParams"));

            debugTechMask_ = renderer->GetMaterialManager()->GetRenderTechniqueInfo("main")->mask_;
        }
        void destroyRenderResources(hRenderer* renderer)
        {
            hMaterialInstance::destroyMaterialInstance(backdropMat_);
            hMaterialInstance::destroyMaterialInstance(fontMat_);
            backdropMat_ = NULL;
            backdropCB_ = NULL;
            fontMat_ = NULL;

            renderer->DestroyVertexBuffer(backdropPlane_);
            backdropPlane_ = NULL;
            renderer->DestroyVertexBuffer(textBuffer_);
            textBuffer_ = NULL;
            renderer->DestroyVertexBuffer(logBuffer_);
            logBuffer_ = NULL;
            consoleFont_ = NULL;
            fontMat_ = NULL;
            debugTechMask_ = 0;
            renderer = NULL;
        }
        void PreRenderUpdate() 
        {
            windowOffset_ += GetVisible() ? -hClock::Delta() : hClock::Delta();
            windowOffset_ = hMax(windowOffset_, 0.f);
            windowOffset_ = hMin(windowOffset_, 1.f);
        }
        void Render(hRenderSubmissionCtx* ctx, hdParameterConstantBlock* instanceCB, const hDebugRenderParams& params) 
        {
            hFixedPoolStackMemoryHeap stackHeap(32*1024, hAlloca(32*1024));
            hFloat intprt = 0.f;
            hInstanceConstants* inst;
            hConstBlockMapInfo map;
            hVertexBufferMapInfo vbmap;
            hFloat fontScale = hSystemConsole::getFontSize();//+(2.f*sin(hClock::elapsed()*.24f));
            hCPUVec2 bottomleft(-(params.rtWidth_/2.f), -(params.rtHeight_/2.f));
            hCPUVec2 bottomright((params.rtWidth_/2.f), -(params.rtHeight_/2.f));
            hCPUVec2 topleft(-(params.rtWidth_/2.f), 1000.f);
            hUint32 prims = 0;
            hFloat* fontParams;
            hFontFormatting formatter(&stackHeap);
            formatter.setScale(fontScale);
            formatter.setFont(consoleFont_);
            formatter.setColour(WHITE);
            formatter.setReserve(256);

            if (windowOffset_ < 1.f) {

                ctx->Map(instanceCB, &map);
                inst = (hInstanceConstants*)map.ptr;
                inst->world_ = hMatrixFunc::identity();
                inst->world_ = hMatrixFunc::Translation(hVec3(0.f, (params.rtHeight_*windowOffset_), 0.f));
                ctx->Unmap(&map);

                //Doesn't need updating every frame, could go in global constant block
                ctx->Map(backdropCB_, &map);
                hCPUVec2* vpsize = (hCPUVec2*)map.ptr;
                vpsize->x = (hFloat)params.rtWidth_;
                vpsize->y = (hFloat)params.rtHeight_;
                ctx->Unmap(&map);

                hMaterialTechnique* tech = backdropMat_->GetTechniqueByMask(debugTechMask_);
                if (!tech) return;
                for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
                    hMaterialTechniquePass* passptr = tech->GetPass(pass);
                    ctx->SetVertexStream(0, backdropPlane_, backdropPlane_->getStride());
                    ctx->SetMaterialPass(passptr);
                    ctx->DrawPrimitive(2, 0);
                }

                {
                    ctx->Map(textBuffer_, &vbmap);
                    formatter.setOutputBuffer(vbmap.ptr_, INPUT_BUFFER_LEN*6*sizeof(hFontVex));
                    formatter.setInputStringBuffer(inputBuffer_, hStrLen(inputBuffer_));
                    formatter.setFormatExtents(FLT_MAX, 0);
                    formatter.setAlignment(hFONT_ALIGN_LEFT|hFONT_ALIGN_TOP);
                    formatter.formatText();
                    formatter.writeTextToBuffer(bottomleft);
                    prims = formatter.getPrimitiveCount();
                    ctx->Unmap(&vbmap);
                }

                /*
                * INput text string
                */
                tech = fontMat_->GetTechniqueByMask(debugTechMask_);
                if (!tech) return;

                ctx->Map(fontCB_, &map);
                fontParams = (hFloat*)map.ptr;
                //colour
                fontParams[0]=.0f;
                fontParams[1]=.0f;
                fontParams[2]=.0f;
                fontParams[3]=1.f;
                //offset
                fontParams[4]=2.f;
                fontParams[5]=-2.f;
                fontParams[6]=0.f;
                fontParams[7]=0.f;
                ctx->Unmap(&map);

                for (hUint32 pass = 0, passcount = tech->GetPassCount() && prims; pass < passcount; ++pass ) {
                    hMaterialTechniquePass* passptr = tech->GetPass(pass);
                    ctx->SetVertexStream(0, textBuffer_, textBuffer_->getStride());
                    ctx->SetMaterialPass(passptr);
                    ctx->SetPrimitiveType(PRIMITIVETYPE_TRILIST);
                    ctx->DrawPrimitive(prims, 0);
                }

                ctx->Map(fontCB_, &map);
                fontParams = (hFloat*)map.ptr;
                //colour
                fontParams[0]=1.f;
                fontParams[1]=1.f;
                fontParams[2]=1.f;
                fontParams[3]=1.f;
                //offset
                fontParams[4]=0.f;
                fontParams[5]=0.f;
                fontParams[6]=0.f;
                fontParams[7]=0.f;
                ctx->Unmap(&map);

                for (hUint32 pass = 0, passcount = tech->GetPassCount() && prims; pass < passcount; ++pass ) {
                    hMaterialTechniquePass* passptr = tech->GetPass(pass);
                    ctx->SetVertexStream(0, textBuffer_, textBuffer_->getStride());
                    ctx->SetMaterialPass(passptr);
                    ctx->SetPrimitiveType(PRIMITIVETYPE_TRILIST);
                    ctx->DrawPrimitive(prims, 0);
                }

                /*
                * Log text string
                */
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


                ctx->Map(fontCB_, &map);
                fontParams = (hFloat*)map.ptr;
                //colour
                fontParams[0]=.0f;
                fontParams[1]=.0f;
                fontParams[2]=.0f;
                fontParams[3]=1.f;
                //offset
                fontParams[4]=2.f;
                fontParams[5]=-2.f;
                fontParams[6]=0.f;
                fontParams[7]=0.f;
                ctx->Unmap(&map);

                for (hUint32 pass = 0, passcount = tech->GetPassCount() && prims; pass < passcount; ++pass ) {
                    hMaterialTechniquePass* passptr = tech->GetPass(pass);
                    ctx->SetVertexStream(0, logBuffer_, logBuffer_->getStride());
                    ctx->SetMaterialPass(passptr);
                    ctx->SetPrimitiveType(PRIMITIVETYPE_TRILIST);
                    ctx->DrawPrimitive(prims, 0);
                }

                ctx->Map(fontCB_, &map);
                fontParams = (hFloat*)map.ptr;
                //colour
                fontParams[0]=1.f;
                fontParams[1]=1.f;
                fontParams[2]=1.f;
                fontParams[3]=1.f;
                //offset
                fontParams[4]=0.f;
                fontParams[5]=0.f;
                fontParams[6]=0.f;
                fontParams[7]=0.f;
                ctx->Unmap(&map);

                for (hUint32 pass = 0, passcount = tech->GetPassCount() && prims; pass < passcount; ++pass ) {
                    hMaterialTechniquePass* passptr = tech->GetPass(pass);
                    ctx->SetVertexStream(0, logBuffer_, logBuffer_->getStride());
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
        hMaterialInstance*       backdropMat_;
        hdParameterConstantBlock* backdropCB_;
        hFont*                   consoleFont_;
        hdParameterConstantBlock* fontCB_;
        hMaterialInstance*       fontMat_;
        hFloat                   windowOffset_;

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
                                     hPublisherContext* evtCtx)
    {
        controllerManager_ = pControllerManager;
        resourceManager_ = pResourceManager;
        renderer_ = renderer;
        vm_ = lua;
        keyboard_ = controllerManager_->GetSystemKeyboard();
        evtCtx_ = evtCtx;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::Destroy()
    {
        consoleWindow_->destroyRenderResources(renderer_);

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

            if (visible_) {
                hUint afterC = cursorPos_;
                hUint32 inputbytes = keyboard_->CharBufferSizeBytes();
                const hChar* inputchars = keyboard_->GetCharBufferData();
                for (hUint i =0; i < inputbytes; ++i) {
                    if (!isprint(inputchars[i])) continue;
                    inputBuffer_[cursorPos_] = inputchars[i];
                    if (cursorPos_ >= hStrLen(inputBuffer_)) inputBuffer_[cursorPos_+1] = 0;
                    cursorPos_ = hMin(cursorPos_+1, INPUT_BUFFER_LEN-2);
                }

                if (keyboard_->GetButton(VK_LEFT).raisingEdge_) {
                    if (cursorPos_ > 0) --cursorPos_;
                }
                else if (keyboard_->GetButton(VK_RIGHT).raisingEdge_) {
                    if (cursorPos_ < hStrLen(inputBuffer_)-1) ++cursorPos_;
                }
                else if (keyboard_->GetButton(VK_BACK).raisingEdge_ && !keyboard_->GetButton(VK_SHIFT).buttonVal_){
                    memmove(inputBuffer_+cursorPos_, inputBuffer_+cursorPos_+1, hStrLen(inputBuffer_+cursorPos_));
                    if (cursorPos_ > 0) --cursorPos_;
                }
                else if (keyboard_->GetButton(VK_DELETE).raisingEdge_) {
                    memmove(inputBuffer_+cursorPos_, inputBuffer_+cursorPos_+1, hStrLen(inputBuffer_+cursorPos_));
                }
                else if (keyboard_->GetButton(VK_HOME).raisingEdge_) {
                    cursorPos_ = 0;
                }
                else if (keyboard_->GetButton(VK_END).raisingEdge_) {
                    cursorPos_ = hStrLen(inputBuffer_)-1;
                }
                else if (keyboard_->GetButton(VK_RETURN).raisingEdge_) {
                    //Run line
                    vm_->ExecuteBuffer(inputBuffer_, hStrLen(inputBuffer_));
                    hZeroMem(inputBuffer_, sizeof(inputBuffer_));
                    inputBuffer_[0] = ' ';
                    cursorPos_ = 0;
                }
            }

            if (keyboard_->GetButton(VK_F4).raisingEdge_)
            {
                visible_ = !visible_;
                hDebugMenuManager::GetInstance()->SetMenuVisiablity("console", visible_);
            }

            hChar prevChar = inputBuffer_[cursorPos_];
            if ((hInt)(hClock::elapsed()*4.f)%2 == 0) {
                inputBuffer_[cursorPos_] = prevChar == '_' ? ' ' : '_';
            }

            hMutexAutoScope mas(&messagesMutex_);
            consoleWindow_->updateConsoleLogString(messageBuffer_, inputBuffer_);

            inputBuffer_[cursorPos_] = prevChar;
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
        
        if (s_consoleOutputCallback){
            s_consoleOutputCallback(string, s_consoleOutputUser);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::ClearLog()
    {
    }

}