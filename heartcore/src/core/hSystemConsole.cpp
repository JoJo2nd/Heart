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
    
    class hConsoleUI : public Gwen::Controls::WindowControl
    {
    public:
        //GWEN_CONTROL_CONSTRUCTOR(hConsoleUI)
        hConsoleUI(Gwen::Controls::Base* parent, hSystemConsole* systemConsole)
            : WindowControl(parent)
            , console_(systemConsole)
        {
            outputList_ = hNEW(GetGlobalHeap(), Gwen::Controls::ListBox)(this);
            outputList_->SetSize(500, 200);
            outputList_->SetHidden(false);

            entry_ = hNEW(GetGlobalHeap(), Gwen::Controls::TextBox)(this);
            entry_->SetText( "" );
            entry_->SetPos(0, 210);
            entry_->SetWidth(500);
            entry_->onReturnPressed.Add( this, &hConsoleUI::OnSubmit );

            SetClosable(hTrue);
            SetDeleteOnClose(false);
            SetTitle("System Console");

            //SizeToChildren(); Doesn't work?
            SetMinimumSize(Gwen::Point(514, 276));
            SetSize(514, 276);
        }

        ~hConsoleUI()
        {

        }

        void AddConsoleString(const hChar* inputStr)
        {
            outputList_->AddItem(inputStr);
            hUint32 c = outputList_->GetTable()->RowCount(0);
            while( c > 15)
            {
                outputList_->SetSelectedRow(outputList_->GetTable()->GetRow(0));
                outputList_->RemoveItem(outputList_->GetTable()->GetRow(0));
                --c;
            }
            outputList_->ScrollToBottom();
        }

    private:

        void OnSubmit( Gwen::Controls::Base* pControl )
        {
            Gwen::Controls::TextBox* textbox = (Gwen::Controls::TextBox*) (pControl);
            console_->ExecuteBuffer(Gwen::Utility::UnicodeToString(textbox->GetText()).c_str());
            textbox->SetText("");
        }

        Gwen::Controls::ListBox* outputList_;
        Gwen::Controls::TextBox* entry_;
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
									 hLuaStateManager* pSquirrel,
									 hResourceManager* pResourceManager,
									 hRenderer* renderer,
                                     hGwenRenderer* uiRenderer)
	{
        controllerManager_ = pControllerManager;
		resourceManager_ = pResourceManager;
        renderer_ = renderer;
		vm_ = pSquirrel;
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

                consoleWindow_ = hNEW(GetDebugHeap(), hConsoleUI)(hDebugMenuManager::GetInstance()->GetDebugCanvas(), this);
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
	void hSystemConsole::LogString( const hChar* inputStr )
	{
        if (consoleWindow_)
        {
            do 
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
            } while (*inputStr);
        }
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