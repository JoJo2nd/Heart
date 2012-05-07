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

    const hUint32		hSystemConsole::FONT_RESOURCE_NAME = hResourceManager::BuildResourceCRC( "ENGINE/FONTS/CONSOLE.FNT" );
	const hUint32		hSystemConsole::CONSOLE_MATERIAL_NAME = hResourceManager::BuildResourceCRC( "ENGINE/EFFECTS/SIMPLECOLOUR.CFX" );
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
		pResourceManager_ = pResourceManager;
        renderer_ = renderer;
		vm_ = pSquirrel;

		screenWidth_ = renderer_->GetWidth();
		screenHeight_ = renderer_->GetHeight();
 
        //////////////////////////////////////////////////////////////////////////
        // Greate resources for displaying the console ///////////////////////////
        //////////////////////////////////////////////////////////////////////////

		hUint16 idx[] =
		{
			0,2,1, 2,3,1, 
		};

        struct Vertex
        {
            hCPUVec3 p;
            hColour  c;
        };

        //////////////////////////////////////////////////////////////////////////
        // Screen is set up as 0,0 in the centre /////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        Vertex verts[] = 
        {
            {hVec3( screenWidth_ / -2.0f, screenHeight_ / 2.0f, 0.1f ), hColour( 0.0f, 0.0f, 0.0f, 0.8f )},
            {hVec3( screenWidth_ / 2.0f, screenHeight_ / 2.0f, 0.1f ) , hColour( 0.0f, 0.0f, 0.0f, 0.8f )},
            {hVec3( screenWidth_ / -2.0f, 0.0f, 0.1f )                , hColour( 0.9f, 0.9f, 0.9f, 1.0f )},
            {hVec3( screenWidth_ / 2.0f, 0.0f, 0.1f )                 , hColour( 0.9f, 0.9f, 0.9f, 1.0f )},
        };
		
		renderer_->CreateIndexBuffer( idx, 6, 0, PRIMITIVETYPE_TRILIST, &indexBuffer_ );	
		renderer_->CreateIndexBuffer( NULL, (INPUT_BUFFER_LEN+MAX_CONSOLE_LOG_SIZE+1)*6, RESOURCEFLAG_DYNAMIC, PRIMITIVETYPE_TRILIST, &IBInput_ );

		renderer_->CreateVertexBuffer( verts, 4, hrVF_XYZ | hrVF_COLOR, 0, &vertexBuffer_ );
		renderer_->CreateVertexBuffer( NULL, (INPUT_BUFFER_LEN+MAX_CONSOLE_LOG_SIZE+1)*6, hrVF_XYZ | hrVF_COLOR | hrVF_1UV, RESOURCEFLAG_DYNAMIC, &VBInput_ );

        renderCamera_.Initialise( renderer_ );
        renderCamera_.SetOrthoParams( (hFloat)screenWidth_, (hFloat)screenHeight_, -0.0f, 1.0f );
        renderCamera_.SetViewMatrix( hMatrixFunc::identity() );
        renderCamera_.SetViewport( hViewport( 0, 0, screenWidth_, screenHeight_ ) );

        rndCtx_ = renderer_->CreateRenderSubmissionCtx();

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSystemConsole::Destroy()
	{
        if (material_ && materialResource_)
        {
            materialResource_->DestroyMaterialInstance(material_);
        }
        HEART_RESOURCE_SAFE_RELEASE(materialResource_);
        HEART_RESOURCE_SAFE_RELEASE(fontResource_);
		if ( indexBuffer_ )
		{
			renderer_->DestroyIndexBuffer(indexBuffer_);
            indexBuffer_ = NULL;
		}
		if ( vertexBuffer_ )
		{
            renderer_->DestroyVertexBuffer(vertexBuffer_);
            vertexBuffer_ = NULL;
		}
		if ( IBInput_ )
		{
            renderer_->DestroyIndexBuffer(IBInput_);
            IBInput_ = NULL;
		}
		if ( VBInput_ )
		{
            renderer_->DestroyVertexBuffer(VBInput_);
            VBInput_ = NULL;
		}
        if (rndCtx_)
        {
            renderer_->DestroyRenderSubmissionCtx(rndCtx_);
            rndCtx_ = NULL;
        }

// 		if ( pRenderCmdBuffer_ )
// 		{
// 			delete pRenderCmdBuffer_;
// 		}
// 		if ( fontResource_.HasData() )
// 		{
// 			fontResource_.Release();
// 		}
// 		if ( textMaterial_.HasData() )
// 		{
// 			textMaterial_.Release();
// 		}
// 		if ( material_.HasData() )
// 		{
// 			material_.Release();
// 		}
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
			if ( pResourceManager_->RequiredResourcesReady() )
			{
                //////////////////////////////////////////////////////////////////////////
                // Get resources for displaying the console //////////////////////////////
                //////////////////////////////////////////////////////////////////////////

                fontResource_ = static_cast< hFont* >( pResourceManager_->mtGetResourceAddRef(FONT_RESOURCE_NAME) );
                materialResource_ = static_cast< hMaterial* >(pResourceManager_->mtGetResourceAddRef(CONSOLE_MATERIAL_NAME));
                material_     = materialResource_->CreateMaterialInstance();
                renderCamera_.SetTechniquePass( renderer_->GetMaterialManager()->GetRenderTechniqueInfo( "main" ) );

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
		if ( loaded_ && shown_ )
		{
			// Set up ortho view
            rndCtx_->SetRendererCamera( &renderCamera_ );
            rndCtx_->SetMaterialInstance( material_ );
            rndCtx_->SetWorldMatrix( Heart::hMatrixFunc::identity() );
            rndCtx_->SetPrimitiveType( Heart::PRIMITIVETYPE_TRILIST );
            rndCtx_->SetVertexStream( 0, vertexBuffer_ );
            rndCtx_->SetIndexStream( indexBuffer_ );

            hUint32 passes = rndCtx_->GetMaterialInstancePasses();
            for ( hUint32 i = 0; i < passes; ++i )
            {
                rndCtx_->BeingMaterialInstancePass( i );
                rndCtx_->DrawIndexedPrimitive( 2, 0 );
                rndCtx_->EndMaterialInstancePass();
            }

			//Render the current Text
			//if ( ( inputDirty_ || logDirty_ ) )
			{
				nInputPrims_ = 0;

				//Render input
				hFontStyle style;
				style.Alignment_ = FONT_ALIGN_LEFT | FONT_ALIGN_TOP;
				style.Colour_ = BLACK;

				fontResource_->SetFontStyle( style );

				if ( nInputChars_ )
				{
                    nInputPrims_ += fontResource_->RenderStringSingleLine( 
                        *IBInput_, 
                        *VBInput_, 
                        hVec2( screenWidth_ / -2.0f, fontResource_->FontBaseLine() ),
                        inputBuffer_,
                        rndCtx_);
				}

				//Render Log
				style.Alignment_ = FONT_ALIGN_LEFT | FONT_ALIGN_BOTTOM;
				style.Colour_ = BLACK;

				fontResource_->SetFontStyle( style );

				if ( !consoleLog_.empty() )
				{
					nInputPrims_ += fontResource_->RenderString( 
													*IBInput_, 
													*VBInput_, 
													hVec2( screenWidth_ / -2.0f, screenHeight_ / 2.0f ),
													hVec2( screenWidth_ / 2.0f,  (hFloat)fontResource_->FontHeight() ),
													consoleLog_.c_str(),
                                                    rndCtx_ );

				}


				inputDirty_ = hFalse;
				logDirty_ = hFalse;
			}
		}

        hdRenderCommandBuffer cb = rndCtx_->SaveToCommandBuffer();
        pRenderer->SubmitRenderCommandBuffer( cb, hTrue );
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
		if ( keyboard_->GetButtonDown( Device::IID_ESCAPE, keyboardAccessKey_ ) )
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

			const hChar* input = keyboard_->GetBufferedText();
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
				if ( keyboard_->GetButtonDown( Device::IID_UPARROW, keyboardAccessKey_ ) )
				{
					++inputHistoryEntry_;
					if ( (hUint32)inputHistoryEntry_ >= inputHistroy_.size() )
					{
						inputHistoryEntry_ = inputHistroy_.size() - 1;
					}
 					strcpy_s( inputBuffer_, INPUT_BUFFER_LEN - 1, inputHistroy_.at( inputHistoryEntry_ ).c_str() );
					nInputChars_ = inputHistroy_.at( inputHistoryEntry_ ).length();
					inputDirty_ = hTrue;
				}
				else if ( keyboard_->GetButtonDown( Device::IID_DOWNARROW, keyboardAccessKey_ ) )
				{
					--inputHistoryEntry_;
					if ( inputHistoryEntry_ < 0 )
					{
						inputHistoryEntry_ = 0;
					}
					strcpy_s( inputBuffer_, INPUT_BUFFER_LEN - 1, inputHistroy_.at( inputHistoryEntry_ ).c_str() );
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