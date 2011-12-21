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

	Channel::Channel( hUint32 channelId ) :
		channelID_( channelId )
		,eventBytes_( 0 )
	{
		postedEvents_.reserve( DEFAULT_CHANNEL_SIZE );
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
		listeners_.push_front( listener );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Channel::RemoveListener( BaseListener* listener )
	{
		listeners_.remove( listener );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Channel::DispatchEvents()
	{
		if ( eventBytes_ )
		{
			hByte* pStart = &(*postedEvents_.begin());
			hByte* pEnd = &postedEvents_[ eventBytes_ ];

			while( pStart < pEnd )
			{
				BaseEvent* pEvent = (BaseEvent*)pStart;

				Listeners::const_iterator iend = listeners_.end();
				for ( Listeners::iterator i = listeners_.begin(); i != iend; ++i )
				{
					if ( (*i)->EventID() == pEvent->EventID() )
					{
						(*i)->PostEvent( pEvent );
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
		Channels::const_iterator iend = channels_.end();
		Channels::iterator i = channels_.begin();
		for ( ; i != iend; ++i )
		{
			hcAssertMsg( (*i)->ChannelID() != channelId, "Trying to add a channel id twice, this is not allowed" );
			if ( (*i)->ChannelID() < channelId )
			{
				break;
			}
		}

		// insert before i
		channels_.insert( i, hNEW ( hGeneralHeap ) Channel( channelId ) );

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void EventManager::RemoveChannel( hUint32 channelId )
	{
		Channels::const_iterator iend = channels_.end();
		for ( Channels::iterator i = channels_.begin(); i != iend; ++i )
		{
			if ( (*i)->ChannelID() == channelId )
			{
				delete *i;
				channels_.erase( i );
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void EventManager::AddListener( hUint32 channelId, BaseListener* listener )
	{
		Channels::const_iterator iend = channels_.end();
		for ( Channels::iterator i = channels_.begin(); i != iend; ++i )
		{
			if ( (*i)->ChannelID() == channelId )
			{
				(*i)->AddListener( listener );
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void EventManager::RemoveListener( hUint32 channelId, BaseListener* listener )
	{
		Channels::const_iterator iend = channels_.end();
		for ( Channels::iterator i = channels_.begin(); i != iend; ++i )
		{
			if ( (*i)->ChannelID() == channelId )
			{
				(*i)->RemoveListener( listener );
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void EventManager::DispatchEvents()
	{
		Channels::const_iterator iend = channels_.end();
		for ( Channels::iterator i = channels_.begin(); i != iend; ++i )
		{
			(*i)->DispatchEvents();
		}
	}

}