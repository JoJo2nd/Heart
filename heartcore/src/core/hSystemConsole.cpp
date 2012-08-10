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
									 hRenderer* renderer )
	{
		keyboard_ = pControllerManager->GetSystemKeyboard();
		resourceManager_ = pResourceManager;
        renderer_ = renderer;
		vm_ = pSquirrel;
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
		if ( !loaded_ )
		{
			if ( resourceManager_->RequiredResourcesReady() )
			{
                //////////////////////////////////////////////////////////////////////////
                // Get resources for displaying the console //////////////////////////////
                //////////////////////////////////////////////////////////////////////////

                static_cast< hFont* >( resourceManager_->mtGetResource(FONT_RESOURCE_NAME) );
                static_cast< hMaterial* >(resourceManager_->mtGetResource(CONSOLE_MATERIAL_NAME));
				loaded_ = hTrue;
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

	void hSystemConsole::Render( hRenderer* pRenderer )
	{
		if ( loaded_ /*&& shown_*/ )
		{
			// Set up ortho view
//             rndCtx_->SetRendererCamera( &renderCamera_ );
//             rndCtx_->SetMaterialInstance( material_ );
//             rndCtx_->SetWorldMatrix( Heart::hMatrixFunc::identity() );
//             rndCtx_->SetPrimitiveType( Heart::PRIMITIVETYPE_TRILIST );
//             rndCtx_->SetVertexStream( 0, vertexBuffer_ );
//             rndCtx_->SetIndexStream( indexBuffer_ );
// 
//             hUint32 passes = rndCtx_->GetMaterialInstancePasses();
//             for ( hUint32 i = 0; i < passes; ++i )
//             {
//                 rndCtx_->BeingMaterialInstancePass( i );
//                 rndCtx_->DrawIndexedPrimitive( 2, 0 );
//                 rndCtx_->EndMaterialInstancePass();
//             }

			//Render the current Text
			//if ( ( inputDirty_ || logDirty_ ) )
			{

				//Render input
				hFontStyle style;
				style.Alignment_ = FONT_ALIGN_LEFT | FONT_ALIGN_TOP;
				style.Colour_ = BLACK;

				//fontResource_->SetFontStyle( style );

				if ( nInputChars_ )
				{
//                     nInputPrims_ += fontResource_->RenderStringSingleLine( 
//                         *IBInput_, 
//                         *VBInput_, 
//                         hVec2( screenWidth_ / -2.0f, fontResource_->GetFontHeight() ),
//                         inputBuffer_,
//                         rndCtx_);
				}

				//Render Log
				style.Alignment_ = FONT_ALIGN_LEFT | FONT_ALIGN_BOTTOM;
				style.Colour_ = BLACK;

				//fontResource_->SetFontStyle( style );

				if ( !consoleLog_.empty() )
				{
// 					nInputPrims_ += fontResource_->RenderString( 
// 													*IBInput_, 
// 													*VBInput_, 
// 													hVec2( screenWidth_ / -2.0f, screenHeight_ / 2.0f ),
// 													hVec2( screenWidth_ / 2.0f,  (hFloat)fontResource_->GetFontHeight() ),
// 													consoleLog_.c_str(),
//                                                     rndCtx_ );

				}


				inputDirty_ = hFalse;
				logDirty_ = hFalse;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSystemConsole::ClearConsoleBuffer()
	{
		nInputChars_ = 0;
		memset( inputBuffer_, 0, INPUT_BUFFER_LEN );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSystemConsole::ExecuteBuffer()
	{
		//add to the log of inputted commands
		//inputBuffer will end with a new line which we dont 
		//want in the history, so remove it then add it back after
		inputBuffer_[ nInputChars_ - 1 ] = 0;
		inputHistroy_.push_front( inputBuffer_ );
		++inputHistoryEntry_;
		
		inputBuffer_[ nInputChars_ - 1 ] = '\n';

		if ( inputHistroy_.size() > MAX_PREV_COMMAND_LOGS )
		{
			inputHistroy_.pop_back();
		}

		inputHistoryEntry_ = -1;

		//Copy the command to the log
		LogString( inputBuffer_ );

		//Run the command
		vm_->ExecuteBuffer( inputBuffer_, nInputChars_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//TODO: make multi thread safe. Lockless queue??
	void hSystemConsole::LogString( const hChar* inputStr )
	{
		consoleLog_ += inputStr;

		if ( consoleLog_.size() > MAX_CONSOLE_LOG_SIZE )
		{
			consoleLog_.erase( 0, consoleLog_.size() - MAX_CONSOLE_LOG_SIZE );
		}

		logDirty_ = hTrue;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSystemConsole::UpdateConsole()
	{
		if ( keyboard_->GetButton( VK_ESCAPE, keyboardAccessKey_ ).raisingEdge_ )
		{
			shown_ = !shown_;
		}

		if ( shown_ && keyboardAccessKey_ == 0 )
		{
			keyboardAccessKey_ = keyboard_->AquireExclusiveAccess();
		}
		else if ( !shown_ && keyboardAccessKey_ != 0 )
		{
			keyboard_->ReleaseExclusiveAccess( keyboardAccessKey_ );
			keyboardAccessKey_ = 0;
		}

		// TODO: buffer checks
		if ( shown_ )
		{
			hUint32 nIC = nInputChars_;

			const hChar* input = keyboard_->GetCharBufferData();
			for ( ; *input != '\0'; ++input )
			{
				if ( nInputChars_ > 0 && iscntrl( *input ) )
				{
					//hcPrintf( "control character entered 0x%02X", *input );
					switch ( *input )
					{
					case 0x7F://DEL
					case 0x08://Back Space
						inputBuffer_[ --nInputChars_ ] = '\0';
						break;
					case 0x0D://Return
						if ( nInputChars_ < INPUT_BUFFER_LEN - 1 )
						{
							inputBuffer_[ nInputChars_++ ] = '\n';
							inputBuffer_[ nInputChars_ ] = '\0';
							ExecuteBuffer();
							ClearConsoleBuffer();
						}
						break;
					}
				}
				else if ( isprint( *input ) )
				{
					if ( nInputChars_ < INPUT_BUFFER_LEN - 2 )
					{
						inputBuffer_[ nInputChars_++ ] = *input;
						inputBuffer_[ nInputChars_ ] = '\0';
					}
				}
			}

			inputDirty_ = nIC != nInputChars_;

			//check for going back thru history
			if ( !inputHistroy_.empty() )
			{
				if ( keyboard_->GetButton( VK_UP, keyboardAccessKey_ ).raisingEdge_ )
				{
					++inputHistoryEntry_;
					if ( (hUint32)inputHistoryEntry_ >= inputHistroy_.size() )
					{
						inputHistoryEntry_ = inputHistroy_.size() - 1;
					}
 					hStrCopy( inputBuffer_, INPUT_BUFFER_LEN - 1, inputHistroy_.at( inputHistoryEntry_ ).c_str() );
					nInputChars_ = inputHistroy_.at( inputHistoryEntry_ ).length();
					inputDirty_ = hTrue;
				}
				else if ( keyboard_->GetButton( VK_UP, keyboardAccessKey_ ).raisingEdge_ )
				{
					--inputHistoryEntry_;
					if ( inputHistoryEntry_ < 0 )
					{
						inputHistoryEntry_ = 0;
					}
					hStrCopy( inputBuffer_, INPUT_BUFFER_LEN - 1, inputHistroy_.at( inputHistoryEntry_ ).c_str() );
					nInputChars_ = inputHistroy_.at( inputHistoryEntry_ ).length();
					inputDirty_ = hTrue;
				}

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
        consoleLog_.clear();
    }

}