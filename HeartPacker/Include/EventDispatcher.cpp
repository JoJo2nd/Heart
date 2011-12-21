/********************************************************************

	filename: 	EventDispatcher.cpp	
	
	Copyright (c) 28:10:2011 James Moran
	
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

#include "PreCompiled.h"
#include "EventDispatcher.h"


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EventDispatcher::RegisterListener( EventListener* listener )
{
    listener->eventDispatcher_ = this;
    listenerList_.push_back( listener );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EventDispatcher::UnregisterListener( EventListener* listener )
{
    listener->eventDispatcher_ = NULL;
    listenerList_.remove( listener );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EventDispatcher::DispatchEvent( const Event& evtName, const void* data, pUint32 len )
{
    Event evt( evtName, data, len );
    for ( ListenerList_t::iterator i = listenerList_.begin(), iend = listenerList_.end();
        i != iend; ++i )
    {
        (*i)->EventRecieve( evt );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void EventListener::UnregisterListener()
{
    if ( eventDispatcher_ )
    {
        eventDispatcher_->UnregisterListener( this );
    }
}
