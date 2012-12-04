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

		static const hUint32								MAX_CONSOLE_LOG_SIZE = 4096;
		static const hUint32								INPUT_BUFFER_LEN = 256;
		static const hUint32								MAX_PREV_COMMAND_LOGS = 32;
		static const hResourceID           					FONT_RESOURCE_NAME;
		static const hResourceID           					CONSOLE_MATERIAL_NAME;

        template< hUint32 t_size >
        class hStringRingBuffer
        {
        public:
            hStringRingBuffer()
            {
                write_ = ring_;
                read_ = ring_;
            }
            void   pushChar(const hChar& x)
            {
                *write_ = x;
                ++write_;
                if (write_ >= (ring_+t_size)) write_ = ring_;
                if (write_ == read_) 
                {
                    ++read_;
                    if (read_ >= (ring_+t_size)) write_ = ring_;
                }
            }
            hChar* getFront() const
            {
                return read_ == write_ ? NULL : write_;
            }
            hChar* getNext(const hChar* c) const
            {
                hcAssertMsg(c >= ring_ && c < ring_+t_size, "parameter doesn't belong to this ring buffer");
                const hChar* r = c;
                ++r;
                if (r == write_) return NULL;//Complete
                return r;
            }
        private:
            hChar* write_, *read_;
            hChar  ring_[t_size];
        };

		void												UpdateConsole();
		void 												ClearConsoleBuffer();
		void												LogString(const hChar* inputStr);

        hRenderer*                                          renderer_;
		hResourceManager*									resourceManager_;
        hDrawCallContext                                    ctx_;
        const hdKeyboard*                                   keyboard_;

        hStringRingBuffer<MAX_CONSOLE_LOG_SIZE>             consoleLog_;                                            
        hChar                                               inputBuffer_[INPUT_BUFFER_LEN];
        hChar                                               prevInputs_[MAX_PREV_COMMAND_LOGS][INPUT_BUFFER_LEN];

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