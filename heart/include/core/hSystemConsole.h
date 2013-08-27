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

typedef void (*hConsoleOutputProc)(const hChar*, void*);

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

    class HEART_DLLEXPORT hSystemConsole
    {
    public:
        hSystemConsole(::hConsoleOutputProc outputcallback, void* user)
            : loaded_( hFalse )
            , consoleWindow_(NULL)
            , visible_(hFalse)
            , cursorPos_(0)
        {
            hZeroMem(inputBuffer_, sizeof(inputBuffer_));
            inputBuffer_[0] = ' ';
            s_consoleOutputCallback = outputcallback;
            s_consoleOutputUser = user;
        }
        ~hSystemConsole()
        {
        }

        void            Initialise( hControllerManager* pControllerManager,
                                    hLuaStateManager* pSquirrel,
                                    hResourceManager* pResourceManager,
                                    hRenderer* renderer,
                                    hPublisherContext* evtCtx);
        void            Destroy();
        void            Update();
        void            ClearLog();
        void            ExecuteBuffer(const hChar* input);
        static void     PrintConsoleMessage(const hChar* string);

        template< hUint32 t_size >
        class HEART_FORCEDLLEXPORT hStringRingBuffer
        {
        public:
            hStringRingBuffer()
            {
                write_ = ring_;
                read_ = ring_;
                newLines_ = 0;
            }
            hStringRingBuffer(const hStringRingBuffer& rhs)
            {
                hMemCpy(ring_, rhs.ring_, sizeof(ring_));
                write_ = ring_+((hUint32)rhs.write_-(hUint32)rhs.ring_);
                read_ = ring_+((hUint32)rhs.read_-(hUint32)rhs.ring_);
                newLines_ = rhs.newLines_;
            }
            hStringRingBuffer& operator = (const hStringRingBuffer rhs)
            {
                swap(*this, rhs);
                return *this;
            }
            void swap(hStringRingBuffer& lhs, hStringRingBuffer& rhs) {
                std::swap(lhs.write_, rhs.write_);
                std::swap(lhs.read_, rhs.read_);
                std::swap(lhs.newLines_, rhs.newLines_);
                std::swap(lhs.ring_, rhs.ring_);
            }
            void   pushChar(const hChar& x)
            {
                if (x == '\n') {
                    ++newLines_;
                }
                *write_ = x;
                ++write_;
                if (write_ >= (ring_+t_size)) {
                    write_ = ring_;
                }
                if (write_ == read_) {
                    if (*read_ == '\n') {
                        --newLines_;
                    }
                    ++read_;
                    if (read_ >= (ring_+t_size)) {
                        read_ = ring_;
                    }
                }
            }
            hChar* getHead() const
            {
                return read_ == write_ ? NULL : read_;
            }
            hChar* getTail() const
            {
                if (read_ == write_) return NULL;
                else if (write_ == ring_) return (hChar*)ring_+(t_size+1);
                else return write_-1;
            }
            const hChar* getNext(const hChar* c) const
            {
                hcAssertMsg(c >= ring_ && c < ring_+t_size, "parameter doesn't belong to this ring buffer");
                const hChar* r = c;
                ++r;
                if (r >= ring_+t_size) r = ring_;
                if (r == write_) return NULL;//Complete
                return r;
            }
            const hChar* getPrev(const hChar* c) const
            {
                hcAssertMsg(c >= ring_ && c < ring_+t_size, "parameter doesn't belong to this ring buffer");
                const hChar* r = c;
                --r;
                if (r <= ring_) r = ring_+(t_size-1);
                if (r == read_) return NULL;//Complete
                return r;
            }
            hUint32 getLineCount() const { return newLines_; }
            void copyToBuffer(hChar* outBuf, hUint32* outsize) 
            {
                if (read_ < write_) {
                    *outsize = ((hUint32)write_-(hUint32)read_);
                    hMemCpy(outBuf, read_, ((hUint32)write_-(hUint32)read_));
                }
                else {
                    *outsize = ((hUint32)(ring_+t_size)-(hUint32)read_);
                    hMemCpy(outBuf, read_, ((hUint32)(ring_+t_size)-(hUint32)read_));
                    hMemCpy(outBuf+(*outsize), ring_, ((hUint32)(write_)-(hUint32)ring_));
                    *outsize += ((hUint32)(write_)-(hUint32)ring_);
                }
            }
        private:
            hChar* write_, *read_;
            hChar  ring_[t_size];
            hUint32 newLines_;
        };
        static void     setFontSize(hFloat size) { s_fontSize = size; }
        static hFloat   getFontSize() { return s_fontSize; }

        static const hUint32        MAX_CONSOLE_LOG_SIZE = 2048;
        static const hUint32        INPUT_BUFFER_LEN = 256;
        typedef hStringRingBuffer<MAX_CONSOLE_LOG_SIZE> hConsoleLogType;

    private:

        static const hUint32        MAX_PREV_COMMAND_LOGS = 32;
        static const hResourceID    FONT_RESOURCE_NAME;
        static const hResourceID    CONSOLE_MATERIAL_NAME;
        static hFloat               s_fontSize;
        static hConsoleOutputProc   s_consoleOutputCallback;
        static void*                s_consoleOutputUser;
        


        void    UpdateConsole();
        void    ClearConsoleBuffer();

        hRenderer*          renderer_;
        hResourceManager*   resourceManager_;
        const hdKeyboard*   keyboard_;
        hBool               visible_;
        hPublisherContext*  evtCtx_;

        hConsoleLogType consoleLog_;
        hUint           cursorPos_;
        hChar           inputBuffer_[INPUT_BUFFER_LEN];
        hChar           prevInputs_[MAX_PREV_COMMAND_LOGS][INPUT_BUFFER_LEN];

        //Display UI
        hControllerManager*     controllerManager_;
        hConsoleUI*             consoleWindow_;

        //command processing
        hLuaStateManager*   vm_;

        //console print mutex
        static hMutex           messagesMutex_;
        static hConsoleLogType  messageBuffer_;  
        static hUint32          msgBufferLen_;
        static hBool            alive_;
        hBool                   loaded_;
    };
}

#endif // SYSTEMCONSOLE_H__