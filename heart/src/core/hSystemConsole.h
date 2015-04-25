/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef SYSTEMCONSOLE_H__
#define SYSTEMCONSOLE_H__ 

#include "base/hTypes.h"
#include "base/hStringID.h"
#include "base/hMemoryUtil.h"
#include "base/hMutex.h"

typedef void (*hConsoleOutputProc)(const hChar*, void*);
#if 0 // !!JM
namespace Heart
{
    class hKeyboard;
    class hLuaStateManager;
    
    class hMaterial;
    class hVertexBuffer;
    class hIndexBuffer;
    class hVertexDeclaration;
    class hConsoleUI;
    class hActionManager;
    class hPublisherContext;
    class hNetHost;

    class hSystemConsole
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

        void            initialise( hActionManager* actionManager,
                                    hLuaStateManager* lua,
                                    hRenderer* renderer,
                                    hPublisherContext* evtCtx,
                                    hNetHost* debugHost);
        void            destroy();
        void            update();
        void            clearLog();
        void            executeBuffer(const hChar* input);
        static void     printConsoleMessage(const hChar* string);

        template< hUint32 t_size >
        class hStringRingBuffer
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
        static hStringID            FONT_RESOURCE_NAME;
        static hStringID            CONSOLE_MATERIAL_NAME;
        static hFloat               s_fontSize;
        static hConsoleOutputProc   s_consoleOutputCallback;
        static void*                s_consoleOutputUser;
        


        void    updateConsole();
        void    clearConsoleBuffer();

        hRenderer*          renderer_;
        hBool               visible_;
        hPublisherContext*  evtCtx_;
        hNetHost*           debugHost_;

        hConsoleLogType consoleLog_;
        hUint           cursorPos_;
        std::string     networkBuffer_;
        hChar           inputBuffer_[INPUT_BUFFER_LEN];
        hChar           prevInputs_[MAX_PREV_COMMAND_LOGS][INPUT_BUFFER_LEN];

        //Display UI
        hActionManager*         actionManager_;
        hConsoleUI*             consoleWindow_;

        //command processing
        hLuaStateManager*   vm_;

        //console print mutex
        static hMutex           messagesMutex_;
        static hConsoleLogType  messageBuffer_;  
        static hUint32          msgBufferLen_;
        static hBool            alive_;
        static std::string      frameMessages_;
        hBool                   loaded_;
    };
}
#endif
#endif // SYSTEMCONSOLE_H__