/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#if 0
#include "core/hSystemConsole.h"
#include "base/hClock.h"
#include "base/hStringUtil.h"
#include "base/hProfiler.h"
#include "base/hProtobuf.h"
#include "debug/hDebugMenuManager.h"
#include "input/hActionManager.h"
#include "lua/hLuaStateManager.h"
#include "render/hDebugDraw.h"
#include "render/hRenderSubmissionContext.h"
#include "network/hNetHost.h"
#include "threading/hMutexAutoScope.h"

namespace Heart
{

    hFloat              hSystemConsole::s_fontSize = 1.f;
    hConsoleOutputProc  hSystemConsole::s_consoleOutputCallback = NULL;
    void*               hSystemConsole::s_consoleOutputUser = NULL;
    std::string         hSystemConsole::frameMessages_;


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    class hConsoleUI : public hDebugMenuBase
    {
    public:
        hConsoleUI(hSystemConsole* systemConsole) 
            : console_(systemConsole)
            , windowOffset_(1.f)
        {

        }

        ~hConsoleUI()
        {

        }

        void InitRenderResources(hRenderer* /*renderer*/)
        {
        }
        void destroyRenderResources(hRenderer* /*renderer*/)
        {
            //renderer = NULL;
        }
        void PreRenderUpdate() 
        {
            windowOffset_ += GetVisible() ? hClock::delta()*4.f : -hClock::delta()*4.f;
            windowOffset_ = hMax(windowOffset_, 0.f);
            windowOffset_ = hMin(windowOffset_, 1.f);
        }
        void Render(hRenderSubmissionCtx* ctx, hRenderBuffer* instanceCB, const hDebugRenderParams& params) 
        {
            static hFloat s_debugFontSize=12;
            hFloat width=(hFloat)params.rtWidth_;
            hFloat height=(hFloat)params.rtHeight_;

            if (windowOffset_ > 0.f) {
                hFloat ytop = height/2.f;
                hFloat ybtm=floorf(height/2.f-height/4.f*windowOffset_);
                hFloat logy=ybtm;
                hVec3 backquad[] = {
                    hVec3(-width/2.f, ytop, 0.f), hVec3(width/2.f, ytop, 0.f), hVec3(-width/2.f, ybtm, 0.f),
                    hVec3(-width/2.f, ybtm, 0.f), hVec3(width/2.f, ytop, 0.f), hVec3(width/2.f, ybtm, 0.f),
                };
                hDebugDraw* dd=hDebugDraw::it();
                dd->begin();
                dd->drawTris(backquad, 6, hColour(0.f, 0.5f, 0.f, 0.3f), eDebugSet_2DNoDepth);
                dd->drawText(hVec3(-width/2.f+1, logy-1, 0.f), inputBuffer_, hColour(0.f, 0.f, 0.f, 1.f));
                dd->drawText(hVec3(-width/2.f, logy, 0.f), inputBuffer_, hColour(1.f, 1.f, 1.f, 1.f));
                logy+=s_debugFontSize;
                if (logSize_ > 0)
                {
                    hChar* logend=log_+logSize_-1;
                    hUint len=0;
                    hFloat logtop=height/2.f;
                    while(logy < logtop && logend > log_) {
                        if (*logend == '\n') {
                            if (len>1) {
                                dd->drawText(hVec3(-width/2.f+1, logy-1, 0.f), logend+1, hColour(0.f, 0.f, 0.f, 1.f), len);
                                dd->drawText(hVec3(-width/2.f, logy, 0.f), logend+1, hColour(1.f, 1.f, 1.f, 1.f), len);
                                logy+=s_debugFontSize;
                            }
                            len=0;
                        }
                        else {
                            ++len;
                        }
                        --logend;
                    }
                }
                dd->end();
            }
        }
        void EndFrameUpdate() {}
        void updateConsoleLogString(hSystemConsole::hConsoleLogType& log, hChar* inputStr) 
        { 
            log.copyToBuffer(log_, &logSize_); 
            hStrCopy(inputBuffer_, INPUT_BUFFER_LEN, inputStr);
        }

    private:

        static const hUint32 INPUT_BUFFER_LEN = hSystemConsole::INPUT_BUFFER_LEN;

        hFloat               windowOffset_;

        hSystemConsole* console_;
        hUint32         logSize_;
        hChar           log_[hSystemConsole::MAX_CONSOLE_LOG_SIZE];
        hChar           inputBuffer_[INPUT_BUFFER_LEN];
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hStringID                       hSystemConsole::FONT_RESOURCE_NAME = hStringID("core/console");
    hStringID                       hSystemConsole::CONSOLE_MATERIAL_NAME = hStringID("core/console");
    hMutex				            hSystemConsole::messagesMutex_;
    hSystemConsole::hConsoleLogType	hSystemConsole::messageBuffer_;
    hUint32				            hSystemConsole::msgBufferLen_ = 0;
    hBool				            hSystemConsole::alive_ = hTrue;


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::initialise( hActionManager* actionManager,
                                     hLuaStateManager* lua,
                                     hRenderer* renderer,
                                     hPublisherContext* evtCtx,
                                     hNetHost* debugHost)
    {
        actionManager_ = actionManager;
        renderer_ = renderer;
        vm_ = lua;
        evtCtx_ = evtCtx;
        debugHost_ = debugHost;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::destroy()
    {
        consoleWindow_->destroyRenderResources(renderer_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::update()
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

                consoleWindow_ = new hConsoleUI(this);
                consoleWindow_->InitRenderResources(renderer_);
                hDebugMenuManager::GetInstance()->RegisterMenu("console",consoleWindow_);
            }
        }
        else
        {
            updateConsole();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::clearConsoleBuffer()
    {
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::executeBuffer(const hChar* input)
    {
        //add to the log of inputted commands
        printConsoleMessage(input);

        //Run the command
        vm_->ExecuteBuffer(input, hStrLen(input));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::updateConsole()
    {
        if (consoleWindow_)
        {
            if (visible_) {
#if 1
                hcPrintf("Stub");
#else
                hUint afterC = cursorPos_;
                hUint32 inputbytes = keyboard_->CharBufferSizeBytes();
                const hChar* inputchars = keyboard_->GetCharBufferData();
                for (hUint i =0; i < inputbytes; ++i) {
                    if (!isprint(inputchars[i])) continue;
                    inputBuffer_[cursorPos_] = inputchars[i];
                    if (cursorPos_ >= hStrLen(inputBuffer_)) inputBuffer_[cursorPos_+1] = 0;
                    cursorPos_ = hMin(cursorPos_+1, INPUT_BUFFER_LEN-2);
                }

                if (keyboard_->GetButton(VK_LEFT).raisingEdge_) {
                    if (cursorPos_ > 0) --cursorPos_;
                }
                else if (keyboard_->GetButton(VK_RIGHT).raisingEdge_) {
                    if (cursorPos_ < hStrLen(inputBuffer_)-1) ++cursorPos_;
                }
                else if (keyboard_->GetButton(VK_BACK).raisingEdge_ && !keyboard_->GetButton(VK_SHIFT).buttonVal_){
                    memmove(inputBuffer_+cursorPos_, inputBuffer_+cursorPos_+1, hStrLen(inputBuffer_+cursorPos_));
                    if (cursorPos_ > 0) --cursorPos_;
                }
                else if (keyboard_->GetButton(VK_DELETE).raisingEdge_) {
                    memmove(inputBuffer_+cursorPos_, inputBuffer_+cursorPos_+1, hStrLen(inputBuffer_+cursorPos_));
                }
                else if (keyboard_->GetButton(VK_HOME).raisingEdge_) {
                    cursorPos_ = 0;
                }
                else if (keyboard_->GetButton(VK_END).raisingEdge_) {
                    cursorPos_ = hStrLen(inputBuffer_)-1;
                }
                else if (keyboard_->GetButton(VK_RETURN).raisingEdge_) {
                    //Run line
                    vm_->ExecuteBuffer(inputBuffer_, hStrLen(inputBuffer_));
                    hZeroMem(inputBuffer_, sizeof(inputBuffer_));
                    inputBuffer_[0] = ' ';
                    cursorPos_ = 0;
                }
#endif
            }

            static hStringID ConsoleAction("Show/Hide Console");
            hInputAction action;
            if (actionManager_->queryAction(0, ConsoleAction, &action) && action.falling_)
            {
                visible_ = !visible_;
                hDebugMenuManager::GetInstance()->SetMenuVisiablity("console", visible_);
            }

            hChar prevChar = inputBuffer_[cursorPos_];
            if ((hInt)(hClock::elapsed()*4.f)%2 == 0) {
                inputBuffer_[cursorPos_] = prevChar == '_' ? ' ' : '_';
            }

            hMutexAutoScope mas(&messagesMutex_);
            consoleWindow_->updateConsoleLogString(messageBuffer_, inputBuffer_);

            proto::ConsoleLogUpdate msg;
            msg.set_logupdate(frameMessages_);
            msg.SerializeToString(&networkBuffer_);
            debugHost_->dispatchReport("console.log", eDispatchReliable, networkBuffer_.c_str(), (hUint)networkBuffer_.size());
            frameMessages_.clear();

            inputBuffer_[cursorPos_] = prevChar;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::printConsoleMessage( const hChar* string )
    {
        hMutexAutoScope mas(&messagesMutex_);

        hUint32 len = hStrLen(string);
        for (hUint32 i = 0; i < len; ++i) {
            messageBuffer_.pushChar(string[i]);
        }

        frameMessages_.append(string);
        
        if (s_consoleOutputCallback){
            s_consoleOutputCallback(string, s_consoleOutputUser);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSystemConsole::clearLog()
    {
    }

}
#endif