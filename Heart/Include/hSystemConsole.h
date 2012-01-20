/********************************************************************
	created:	2010/09/04
	created:	4:9:2010   20:41
	filename: 	hSystemConsole.h	
	author:		James
	
	purpose:	A id engine style console
*********************************************************************/

#ifndef SYSTEMCONSOLE_H__
#define SYSTEMCONSOLE_H__

#include "hTypes.h"
#include "hFont.h"
#include "hRendererCamera.h"

namespace Heart
{
	class hControllerManager;
	class hKeyboard;
	class hLuaStateManager;
	class hRenderer;
	class hMaterial;
	class hVertexBuffer;
	class hIndexBuffer;
	class hVertexDeclaration;

	class hSystemConsole
	{
	public:
		hSystemConsole()
			: keyboard_( NULL )
			,shown_( hFalse )
			,nInputChars_( 0 )
			,screenWidth_( 0 )
			,screenHeight_( 0 )
			,nInputPrims_( 0 )
			,keyboardAccessKey_( 0 )
			,inputDirty_( hFalse )
			,logDirty_( hFalse )
			,loaded_( hFalse )
		{
			memset( inputBuffer_, 0, INPUT_BUFFER_LEN );
		}
		~hSystemConsole()
		{
		}

		void												Initialise( hControllerManager* pControllerManager,
																		hLuaStateManager* pSquirrel,
																		hResourceManager* pResourceManager,
																		hRenderer* pRenderer );
		void												Destroy();

		void												Update();
		void												Render( hRenderer* pRenderer );

		static void											PrintConsoleMessage( const hChar* string );

	private:

		static const hUint32								MAX_CONSOLE_LOG_SIZE = 4086;
		static const hUint32								INPUT_BUFFER_LEN = 128;
		static const hUint32								MAX_PREV_COMMAND_LOGS = 32;
		static const hUint32								FONT_RESOURCE_NAME;
		static const hUint32								CONSOLE_MATERIAL_NAME;

		void												UpdateConsole();
		void 												ClearConsoleBuffer();
		void 												ExecuteBuffer();
		void												LogString( const hChar* inputStr );

        hRenderer*                                          renderer_;
		hResourceManager*									pResourceManager_;
        hRendererCamera                                     renderCamera_;
        hRenderSubmissionCtx*                               rndCtx_;
		hFont*						                        fontResource_;
		//Background rendering 
		hMaterialInstance* 					                material_;
		hIndexBuffer*				                        indexBuffer_;
		hVertexBuffer* 				                        vertexBuffer_;
		//
		hUint32												screenWidth_;
		hUint32												screenHeight_;
		const hKeyboard*									keyboard_;			
		hBool												shown_;
		hUint32												keyboardAccessKey_;
		hUint32												nInputChars_;
		hChar												inputBuffer_[ INPUT_BUFFER_LEN ];
		string												consoleLog_;
		hInt32												inputHistoryEntry_;
		deque< string >										inputHistroy_;
		bool												inputDirty_;
		bool												logDirty_;
		//command processing
		hLuaStateManager*									vm_;
		//input rendering vars
		hUint32												nInputPrims_;
		hIndexBuffer*				                        IBInput_;
		hVertexBuffer* 				                        VBInput_;

		//console print mutex
		static hMutex										messagesMutex_;
		static string										awaitingMessages_;
		static hBool										alive_;
		hBool												loaded_;
	};
}

#endif // SYSTEMCONSOLE_H__