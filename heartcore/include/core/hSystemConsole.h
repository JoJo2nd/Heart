/********************************************************************

	filename: 	hSystemConsole.h	
	
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

#ifndef SYSTEMCONSOLE_H__
#define SYSTEMCONSOLE_H__

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
    class hConsoleUI;

    class hSystemConsole
	{
	public:
		hSystemConsole()
			: loaded_( hFalse )
            , consoleWindow_(NULL)
		{
		}
		~hSystemConsole()
		{
		}

        void												Initialise( hControllerManager* pControllerManager,
                                                                        hLuaStateManager* pSquirrel,
                                                                        hResourceManager* pResourceManager,
                                                                        hRenderer* renderer,
                                                                        hGwenRenderer* uiRenderer );
		void												Destroy();
		void												Update();
        void                                                ClearLog();
        void 												ExecuteBuffer(const hChar* input);
		static void											PrintConsoleMessage( const hChar* string );

	private:

		static const hUint32								MAX_CONSOLE_LOG_SIZE = 4086;
		static const hUint32								INPUT_BUFFER_LEN = 128;
		static const hUint32								MAX_PREV_COMMAND_LOGS = 32;
		static const hResourceID           					FONT_RESOURCE_NAME;
		static const hResourceID           					CONSOLE_MATERIAL_NAME;

		void												UpdateConsole();
		void 												ClearConsoleBuffer();
		void												LogString( const hChar* inputStr );

        hRenderer*                                          renderer_;
		hResourceManager*									resourceManager_;
        hDrawCallContext                                    ctx_;
        const hdKeyboard*                                   keyboard_;

        //Display UI
        hControllerManager*                                 controllerManager_;
        hConsoleUI*                                         consoleWindow_;

		//command processing
		hLuaStateManager*									vm_;

		//console print mutex
		static hMutex										messagesMutex_;
		static hString										awaitingMessages_;
		static hBool										alive_;
		hBool												loaded_;
	};
}

#endif // SYSTEMCONSOLE_H__