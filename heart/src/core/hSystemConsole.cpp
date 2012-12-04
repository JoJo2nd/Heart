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

            renderer->CreateVertexBuffer(consolePlane, 6, layout, hStaticArraySize(layout), 0, GetDebugHeap(), &backdropPlane_);
            backdropMat_ = static_cast< hMaterial* >(resmanager->mtGetResource("CORE.DEBUG_MAT"));

            debugTechMask_ = renderer->GetMaterialManager()->GetRenderTechniqueInfo("main")->mask_;
        }
        void PreRenderUpdate() {}
        void Render(hRenderSubmissionCtx* ctx, hdParameterConstantBlock* instanceCB) 
        {
            hInstanceConstants* inst;
            hConstBlockMapInfo map;

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
                    /*ctx->SetInputStreams()*/
                    ctx->SetMaterialPass(passptr);
                    ctx->SetPrimitiveType(PRIMITIVETYPE_TRILIST);
                    ctx->DrawPrimitive(2, 0);
                }
            }
        }
        void EndFrameUpdate() {}

    private:

        hUint32                  debugTechMask_;
        hVertexBuffer*           textBuffer_;
        hVertexBuffer*           backdropPlane_;
        hMaterial*               backdropMat_;
        hSystemConsole*          console_;
    };


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    const hResourceID   hSystemConsole::FONT_RESOURCE_NAME = hResourceManager::BuildResourceID( "CORE.CONSOLE" );
	const hResourceID   hSystemConsole::CONSOLE_MATERIAL_NAME = hResourceManager::BuildResourceID( "CORE.FONT_MAT" );
	hMutex				hSystemConsole::messagesMutex_;
	hString				hSystemConsole::awaitingMessages_;
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
		messagesMutex_.Lock();
		alive_ = hFalse;
		awaitingMessages_.clear();
		awaitingMessages_.~hString();
		messagesMutex_.Unlock();
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
			messagesMutex_.Lock();
			if ( !awaitingMessages_.empty() )
			{
				LogString( awaitingMessages_.c_str() );
				awaitingMessages_.clear();
			}
			messagesMutex_.Unlock();
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
        LogString(input);

		//Run the command
		vm_->ExecuteBuffer(input, hStrLen(input));
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//TODO: make multi thread safe. Lockless queue??
	void hSystemConsole::LogString(const hChar* inputStr)
	{
        /*do 
        {
            //nasty const cast
            hChar* s = (hChar*)hStrChr(inputStr, '\n');
            if (s) *s = 0;
            consoleWindow_->AddConsoleString(inputStr);
            if (s) 
            {
                *s = '\n';
                inputStr = s+1;
            }
            else
            {
                return;
            }
        } while (*inputStr);*/
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
                //consoleWindow_->SetHidden(!consoleWindow_->Hidden());
                hDebugMenuManager::GetInstance()->SetMenuVisiablity("console", hTrue);
            }
        }
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSystemConsole::PrintConsoleMessage( const hChar* string )
	{
		messagesMutex_.Lock();
		if ( alive_ )
		{
			awaitingMessages_ += string;
		}
		messagesMutex_.Unlock();
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::ClearLog()
    {
    }

}