/********************************************************************

    filename: 	DeviceEvent.h	
    
    Copyright (c) 15:2:2012 James Moran
    
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
#ifndef DEVICEEVENT_H__
#define DEVICEEVENT_H__


namespace Heart
{
    class hThreadEvent
    {
    public:
        hThreadEvent(hBool autoreset=hTrue) :
            Event_( NULL )
        {
            Event_ = CreateEvent( NULL, !autoreset, FALSE, NULL );
            hcAssert( Event_ != NULL );
        }

        virtual ~hThreadEvent()
        {
            hcAssert( Event_ != NULL );
            CloseHandle( Event_ );
        }

        void Wait()
        {
            hcAssert( Event_ != NULL );
            WaitForSingleObject( Event_, INFINITE );
        }

        hBool TryWait()
        {
            hcAssert( Event_ != NULL );
            DWORD ret = WaitForSingleObject( Event_, 0 );
            return ret == WAIT_OBJECT_0 ? hTrue : hFalse;
        }

        void Signal()
        {
            hcAssert( Event_ != NULL );
            SetEvent( Event_ );
        }

        void reset() {
            hcAssert( Event_ != NULL );
            ResetEvent(Event_);
        }

    private:

        HANDLE		Event_;
    };
}

#endif // DEVICEEVENT_H__