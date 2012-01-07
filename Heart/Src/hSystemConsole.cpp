/********************************************************************
	created:	2010/09/21
	created:	21:9:2010   20:57
	filename: 	hSystemConsole.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hSystemConsole.h"
#include "hControllerManager.h"
#include "hResource.h"
#include "hResourceManager.h"
#include "hRenderer.h"
#include "hTexture.h"
#include "hRenderCommon.h"
#include "hVertexBuffer.h"
#include "hIndexBuffer.h"
#include "hLuaStateManager.h"

namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	const hChar*		hSystemConsole::FONT_RESOURCE_NAME = "engine/fonts/console.fnt";
	const hChar*		hSystemConsole::TEXT_MATERIAL_NAME = "engine/materials/basic2dtex.mat";
	const hChar*		hSystemConsole::CONSOLE_MATERIAL_NAME = "engine/materials/consoleback.mat";
	hMutex				hSystemConsole::messagesMutex_;
	string				hSystemConsole::awaitingMessages_;
	hBool				hSystemConsole::alive_ = hTrue;


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSystemConsole::Initialise( hControllerManager* pControllerManager,
									hLuaStateManager* pSquirrel,
									hResourceManager* pResourceManager,
									hRenderer* pRenderer )
	{
		keyboard_ = pControllerManager->GetSystemKeyboard();
		pResourceManager_ = pResourceManager;
		vm_ = pSquirrel;

		screenWidth_ = pRenderer->Width();
		screenHeight_ = pRenderer->Height();
 
// 		//////////////////////////////////////////////////////////////////////////
// 		// Get resources for displaying the console //////////////////////////////
// 		//////////////////////////////////////////////////////////////////////////
// 
// 		pResourceManager_->GetResource( FONT_RESOURCE_NAME, fontResource_ );
// 		pResourceManager_->GetResource( TEXT_MATERIAL_NAME, textMaterial_ );
// 		pResourceManager_->GetResource( CONSOLE_MATERIAL_NAME, material_ );
// 
// 		pRenderer->GetVertexDeclaration( pVertexDecl_, 
// 											hrVF_XYZ | hrVF_COLOR );
// 
// 		pRenderer->GetVertexDeclaration( pTextVertexDecl_,
// 											hrVF_XYZ | hrVF_COLOR | hrVF_1UV );
// 
// 		hUint16 idx[] =
// 		{
// 			0,2,1, 2,3,1, 
// 		};
// 		
// 		pRenderer->CreateIndexBuffer( indexBuffer_, idx, 6, 0, PRIMITIVETYPE_TRILIST );
// 		
// 		pRenderer->CreateIndexBuffer( IBInput_, NULL, (INPUT_BUFFER_LEN+MAX_CONSOLE_LOG_SIZE+1)*6, DYNAMIC, PRIMITIVETYPE_TRILIST );
// 
// 		pRenderer->CreateVertexBuffer( vertexBuffer_, 4, pVertexDecl_, 0 );
// 
// 		pRenderer->CreateVertexBuffer( VBInput_, (INPUT_BUFFER_LEN+MAX_CONSOLE_LOG_SIZE+1)*6, pTextVertexDecl_, DYNAMIC );
// 
// 		//////////////////////////////////////////////////////////////////////////
// 		// Screen is set up as 0,0 in the centre /////////////////////////////////
// 		//////////////////////////////////////////////////////////////////////////
// 		vertexBuffer_->Lock();
// 		//top left
// 		vertexBuffer_->SetElement( 0, hrVE_XYZ, hVec3( screenWidth_ / -2.0f, screenHeight_ / 2.0f, 0.0f ) );
// 		vertexBuffer_->SetElement( 0, hrVE_COLOR, (hUint32)hColour( 0.0f, 0.0f, 0.0f, 0.8f ) );
// 
// 		//top right
// 		vertexBuffer_->SetElement( 1, hrVE_XYZ, hVec3( screenWidth_ / 2.0f, screenHeight_ / 2.0f, 0.0f ) );
// 		vertexBuffer_->SetElement( 1, hrVE_COLOR, (hUint32)hColour( 0.0f, 0.0f, 0.0f, 0.8f ) );
// 
// 		//bottom left
// 		vertexBuffer_->SetElement( 2, hrVE_XYZ, hVec3( screenWidth_ / -2.0f, 0.0f, 0.0f ) );
// 		vertexBuffer_->SetElement( 2, hrVE_COLOR, (hUint32)hColour( 0.6f, 0.6f, 0.6f, 0.8f ) );
// 
// 		//bottom right
// 		vertexBuffer_->SetElement( 3, hrVE_XYZ, hVec3( screenWidth_ / 2.0f, 0.0f, 0.0f ) );
// 		vertexBuffer_->SetElement( 3, hrVE_COLOR, (hUint32)hColour( 0.6f, 0.6f, 0.6f, 0.8f ) );
// 
// 
// 		vertexBuffer_->Unlock();
// 
// 		pRenderCmdBuffer_ = pRenderer->NewRenderCommandList();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSystemConsole::Destroy()
	{
// 		if ( pRenderCmdBuffer_ )
// 		{
// 			delete pRenderCmdBuffer_;
// 		}
		if ( indexBuffer_.HasData() )
		{
			indexBuffer_.Release();
		}
		if ( vertexBuffer_.HasData() )
		{
			vertexBuffer_.Release();
		}
		if ( IBInput_.HasData() )
		{
			IBInput_.Release();
		}
		if ( VBInput_.HasData() )
		{
			VBInput_.Release();
		}
		if ( fontResource_.HasData() )
		{
			fontResource_.Release();
		}
		if ( textMaterial_.HasData() )
		{
			textMaterial_.Release();
		}
		if ( material_.HasData() )
		{
			material_.Release();
		}
		messagesMutex_.Lock();
		alive_ = hFalse;
		awaitingMessages_.clear();
		awaitingMessages_.~string();
		messagesMutex_.Unlock();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSystemConsole::Update()
	{

// 		if ( !loaded_ )
// 		{
// 			loaded_ = hTrue;
// 			if ( !fontResource_.IsLoaded() || !textMaterial_.IsLoaded() || !material_.IsLoaded() )
// 			{
// 				loaded_ = hFalse;
// 			}
// 		}
// 		else
// 		{
// 			messagesMutex_.Lock();
// 			if ( !awaitingMessages_.empty() )
// 			{
// 				LogString( awaitingMessages_.c_str() );
// 				awaitingMessages_.clear();
// 			}
// 			messagesMutex_.Unlock();
// 			UpdateConsole();
// 		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSystemConsole::Render( hRenderer* pRenderer )
	{
// 		if ( loaded_ && shown_ )
// 		{
// 			// Set up ortho view
// 			hViewport vp = { 0, 0, pRenderer->Width(), pRenderer->Height() };
// 			hMatrix	idnt,wvp;
// 
// 			hMatrix::identity( &idnt );
// 			hMatrix::orthoProj( &wvp, (hFloat)screenWidth_, (hFloat)screenHeight_, -0.0f, 1.0f );
// 
// 			//Render the background to the console
// 			pRenderer->NewRenderCommand< Cmd::SetViewport >( vp );
// 			pRenderer->NewRenderCommand< Cmd::SetWorldMatrix >( &idnt );
// 			pRenderer->NewRenderCommand< Cmd::SetViewMatrix >( &idnt );
// 			pRenderer->NewRenderCommand< Cmd::SetProjectionMatrix >( &wvp );
// 			pRenderer->NewRenderCommand< Cmd::SetMaterial >( material_ );
// 			pRenderer->NewRenderCommand< Cmd::SetStreams >( indexBuffer_, vertexBuffer_ );
// 			pRenderer->NewRenderCommand< Cmd::DrawPrimative >( 2 );
// 
// 			//Render the current Text
// 			if ( ( inputDirty_ || logDirty_ ) )
// 			{
// 				nInputPrims_ = 0;
// 				IBInput_->Lock();
// 				VBInput_->Lock();
// 
// 				//Render input
// 				hFontStyle style;
// 				style.Alignment_ = FONT_ALIGN_LEFT | FONT_ALIGN_TOP;
// 				style.Colour_ = hColour( 1.0f, 1.0f, 1.0f, 1.0f );
// 
// 				fontResource_->SetFontStyle( style );
// 
// 				if ( nInputChars_ )
// 				{
// 					nInputPrims_ = fontResource_->RenderString( 
// 													*IBInput_, 
// 													*VBInput_, 
// 													hVec2( screenWidth_ / -2.0f, (hFloat)fontResource_->FontHeight() ),
// 													hVec2( screenWidth_ / 2.0f, 0.0f ),
// 													inputBuffer_,
// 													textMaterial_,
// 													pRenderCmdBuffer_ );
// 				}
// 
// 				//Render Log
// 				style.Alignment_ = FONT_ALIGN_LEFT | FONT_ALIGN_BOTTOM;
// 				style.Colour_ = hColour( 0.7f, 0.7f, 0.7f, 0.7f );;
// 
// 				fontResource_->SetFontStyle( style );
// 
// 				if ( !consoleLog_.empty() )
// 				{
// 					nInputPrims_ += fontResource_->RenderString( 
// 													*IBInput_, 
// 													*VBInput_, 
// 													hVec2( screenWidth_ / -2.0f, screenHeight_ / 2.0f ),
// 													hVec2( screenWidth_ / 2.0f,  (hFloat)fontResource_->FontHeight() ),
// 													consoleLog_.c_str(),
// 													textMaterial_,
// 													pRenderCmdBuffer_ );
// 				}
// 
// 				IBInput_->Unlock();
// 				VBInput_->Unlock();
// 
// 				inputDirty_ = hFalse;
// 				logDirty_ = hFalse;
// 			}
// 
// 			//Render the log
// 			if ( nInputPrims_ )
// 			{
// 				pRenderer->DrawRenderCommandList( pRenderCmdBuffer_ );
// 			}
// 		}
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
		if ( keyboard_->GetButtonDown( Device::IID_TAB, keyboardAccessKey_ ) )
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

}