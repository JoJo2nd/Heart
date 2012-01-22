/********************************************************************
	created:	2010/08/21
	created:	21:8:2010   13:49
	filename: 	Event.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hEventManager.h"

namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Channel::Channel( hUint32 channelId ) 
        : channelID_(channelId)
		, eventBytes_(0)
        , listenerCount_(0)
	{
		postedEvents_.Resize( DEFAULT_CHANNEL_SIZE );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Channel::~Channel()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Channel::AddListener( BaseListener* listener )
	{
        if ( listenerCount_ >= listeners_.GetSize() )
        {
            listeners_.PushBack( listener );
            listenerCount_ = listeners_.GetSize();
        }
        else
        {
            listeners_[listenerCount_++] = listener;
        }
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Channel::RemoveListener( BaseListener* listener )
	{
		for ( hUint32 i = 0; i < listenerCount_; ++i )
        {
            if ( listeners_[i] == listener )
            {
                listeners_[i] = listeners_[--listenerCount_];
                return;
            }
        }
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Channel::DispatchEvents()
	{
		if ( eventBytes_ )
		{
			hByte* pStart = postedEvents_;
			hByte* pEnd = &postedEvents_[eventBytes_];

			while( pStart < pEnd )
			{
				BaseEvent* pEvent = (BaseEvent*)pStart;

				for ( hUint32 i = 0; i != listenerCount_; ++i )
				{
					if ( listeners_[i]->EventID() == pEvent->EventID() )
					{
						listeners_[i]->PostEvent( pEvent );
					}
				}

				pStart += pEvent->Size();
			}

			eventBytes_ = 0;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	EventManager::EventManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	EventManager::~EventManager()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void EventManager::AddChannel( hUint32 channelId )
	{
        Channel* i = NULL;
        for ( i = channels_.GetHead(); i != NULL; i = i->GetNext() )
        {
			hcAssertMsg( i->ChannelID() != channelId, "Trying to add a channel id twice, this is not allowed" );
			if ( i->ChannelID() < channelId )
			{
				break;
			}
		}

		// insert before i
		channels_.InsertBefore( i, hNEW ( hGeneralHeap ) Channel( channelId ) );

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void EventManager::RemoveChannel( hUint32 channelId )
	{
        Channel* i = NULL;
        for ( i = channels_.GetHead(); i != NULL; i = i->GetNext() )
        {
			if ( i->ChannelID() == channelId )
			{
				break;
			}
		}

        Channel* removed = channels_.Remove( i );
        hDELETE removed;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void EventManager::AddListener( hUint32 channelId, BaseListener* listener )
	{
        for ( Channel* i = channels_.GetHead(); i != NULL; i = i->GetNext() )
        {
			if ( i->ChannelID() == channelId )
			{
				i->AddListener( listener );
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void EventManager::RemoveListener( hUint32 channelId, BaseListener* listener )
	{
        for ( Channel* i = channels_.GetHead(); i != NULL; i = i->GetNext() )
        {
			if ( i->ChannelID() == channelId )
			{
				i->RemoveListener( listener );
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void EventManager::DispatchEvents()
	{
        for ( Channel* i = channels_.GetHead(); i != NULL; i = i->GetNext() )
        {
			i->DispatchEvents();
		}
	}

}