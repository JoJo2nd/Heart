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

    hFloat              hSystemConsole::s_fontSize = .75f;
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
            , consoleFont_(NULL)
            , logMat_(NULL)
            , windowOffset_(1.f)
        {

        }

        ~hConsoleUI()
        {

        }

        void InitRenderResources(hRenderer* renderer)
        {
            hRenderMaterialManager* matManager=renderer->GetMaterialManager();
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

            hSamplerStateDesc ssdesc;
            ssdesc.filter_        = SSV_LINEAR;
            ssdesc.addressU_      = SSV_CLAMP;
            ssdesc.addressV_      = SSV_CLAMP;
            ssdesc.addressW_      = SSV_CLAMP;
            ssdesc.mipLODBias_    = 0;
            ssdesc.maxAnisotropy_ = 16;
            ssdesc.borderColour_  = WHITE;
            ssdesc.minLOD_        = -FLT_MAX;
            ssdesc.maxLOD_        = FLT_MAX;  
            fontSamplerState_=renderer->createSamplerState(ssdesc);

            renderer->createVertexBuffer(consolePlane, 6, layout, (hUint32)hStaticArraySize(layout)-1, 0, GetDebugHeap(), &backdropPlane_);
            hMaterial* backdropMat = matManager->getConsoleMat();
            backdropMat_ = backdropMat->createMaterialInstance(0);
            backdropMat_->bindInputStreams(PRIMITIVETYPE_TRILIST, NULL, &backdropPlane_, 1);

            renderer->createVertexBuffer(textBuffer_, INPUT_BUFFER_LEN*6, layout, (hUint32)hStaticArraySize(layout), RESOURCEFLAG_DYNAMIC, GetDebugHeap(), &textBuffer_);
            renderer->createVertexBuffer(logBuffer_, hSystemConsole::MAX_CONSOLE_LOG_SIZE*6, layout, (hUint32)hStaticArraySize(layout), RESOURCEFLAG_DYNAMIC, GetDebugHeap(), &logBuffer_);
            consoleFont_=hNEW(GetGlobalHeap(), hFont)(GetGlobalHeap());
            hRenderUtility::createDebugFont(renderer, consoleFont_, &consoleTex_, GetGlobalHeap());
            hShaderResourceViewDesc srvd;
            hZeroMem(&srvd, sizeof(srvd));
            srvd.format_=consoleTex_->getTextureFormat();
            srvd.resourceType_=eRenderResourceType_Tex2D;
            srvd.tex2D_.topMip_=0;
            srvd.tex2D_.mipLevels_=~0;
            renderer->createShaderResourceView(consoleTex_, srvd, &consoleTexSRV_);
            hMaterial* fontMat=matManager->getDebugFontMat();
            hShaderParameterID fontsamplerid=hCRC32::StringCRC("fontSampler");
            logMat_ = fontMat->createMaterialInstance(0);
            logMat_->bindInputStreams(PRIMITIVETYPE_TRILIST, NULL, &logBuffer_, 1);
            logMat_->bindResource(fontsamplerid, consoleTexSRV_);
            logMat_->bindSampler(fontsamplerid, fontSamplerState_);
            inputMat_=fontMat->createMaterialInstance(hMatInst_DontInstanceConstantBuffers);
            inputMat_->bindInputStreams(PRIMITIVETYPE_TRILIST, NULL, &textBuffer_, 1);

            fontCB_ = logMat_->GetParameterConstBlock(hCRC32::StringCRC("FontParams"));
            inputMat_->bindConstanstBuffer(hCRC32::StringCRC("FontParams"), fontCB_);

            debugTechMask_ = renderer->GetMaterialManager()->GetRenderTechniqueInfo("main")->mask_;
        }
        void destroyRenderResources(hRenderer* renderer)
        {
            hRenderUtility::destroyDebugFont(renderer, consoleFont_, consoleTex_);
            consoleTex_=NULL;
            hDELETE_SAFE(GetGlobalHeap(), consoleFont_);
            hMaterialInstance::destroyMaterialInstance(backdropMat_);
            hMaterialInstance::destroyMaterialInstance(logMat_);
            hMaterialInstance::destroyMaterialInstance(inputMat_);
            fontSamplerState_->DecRef();
            fontSamplerState_=NULL;
            backdropMat_ = NULL;
            logMat_ = NULL;
            consoleTexSRV_->DecRef();
            consoleTexSRV_=NULL;

            backdropPlane_->DecRef();
            backdropPlane_ = NULL;
            textBuffer_->DecRef();
            textBuffer_ = NULL;
            logBuffer_->DecRef();
            logBuffer_ = NULL;
            consoleFont_ = NULL;
            logMat_ = NULL;
            debugTechMask_ = 0;
            renderer = NULL;
        }
        void PreRenderUpdate() 
        {
            windowOffset_ += GetVisible() ? -hClock::Delta() : hClock::Delta();
            windowOffset_ = hMax(windowOffset_, .5f);
            windowOffset_ = hMin(windowOffset_, 1.f);
        }
        void Render(hRenderSubmissionCtx* ctx, hParameterConstantBlock* instanceCB, const hDebugRenderParams& params) 
        {
            hFixedPoolStackMemoryHeap stackHeap(32*1024, hAlloca(32*1024));
            hFloat intprt = 0.f;
            hInstanceConstants* inst;
            hConstBlockMapInfo map;
            hVertexBufferMapInfo vbmap;
            hFloat fontScale = hSystemConsole::getFontSize();
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
                inst->world_ = hMatrixFunc::translation(hVec3(0.f, (params.rtHeight_*windowOffset_), 0.f));
                ctx->Unmap(&map);

                hMaterialTechnique* tech = backdropMat_->GetTechniqueByMask(debugTechMask_);
                if (!tech) return;
                for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
                    hMaterialTechniquePass* passptr = tech->GetPass(pass);
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
                tech = inputMat_->GetTechniqueByMask(debugTechMask_);
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
                    ctx->SetMaterialPass(passptr);
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
                    ctx->SetMaterialPass(passptr);
                    ctx->DrawPrimitive(prims, 0);
                }

                /*
                * Log text string
                */
                tech = logMat_->GetTechniqueByMask(debugTechMask_);
                if (!tech) return;

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

                tech = logMat_->GetTechniqueByMask(debugTechMask_);
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
                    ctx->SetMaterialPass(passptr);
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
                    ctx->SetMaterialPass(passptr);
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
        hFont*                   consoleFont_;
        hTexture*                consoleTex_;
        hShaderResourceView*     consoleTexSRV_;
        hSamplerState*           fontSamplerState_;
        hParameterConstantBlock* fontCB_;
        hMaterialInstance*       logMat_;
        hMaterialInstance*       inputMat_;
        hFloat                   windowOffset_;

        hSystemConsole* console_;
        hUint32         logSize_;
        hChar           log_[hSystemConsole::MAX_CONSOLE_LOG_SIZE];
        hChar           inputBuffer_[INPUT_BUFFER_LEN];
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hResourceID               hSystemConsole::FONT_RESOURCE_NAME = hResourceManager::BuildResourceID( "CORE.CONSOLE" );
    const hResourceID               hSystemConsole::CONSOLE_MATERIAL_NAME = hResourceManager::BuildResourceID( "CORE.FONT_MAT" );
    hMutex				            hSystemConsole::messagesMutex_;
    hSystemConsole::hConsoleLogType	hSystemConsole::messageBuffer_;
    hUint32				            hSystemConsole::msgBufferLen_ = 0;
    hBool				            hSystemConsole::alive_ = hTrue;


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
                consoleWindow_->InitRenderResources(renderer_);
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