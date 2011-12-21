/********************************************************************
	created:	2008/10/11
	filename: 	Event.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef EVENT_H__
#define EVENT_H__

#include "huFunctor.h"
#include "HeartSTL.h"

namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class BaseEvent
	{
	public:
		virtual hUint32		EventID() const = 0;
		virtual hUint32		Size() const = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	template < hUint32 id, typename _Ty >
	class Event : public BaseEvent
	{
	public:
		static const hUint32 EVENT_ID = id;
		typedef _Ty PayloadType;

		Event()
		{}
		template< typename _P1 >
		Event( _P1 p1 ) : 
			payload_( p1 )
		{}
		template< typename _P1, typename _P2 >
		Event( _P1 p1, _P2 p2 ) : 
		payload_( p1, p2 )
		{}
		template< typename _P1, typename _P2, typename _P3 >
		Event( _P1 p1, _P2 p2, _P3 p3 ) : 
		payload_( p1, p2, p3 )
		{}
		template< typename _P1, typename _P2, typename _P3, typename _P4 >
		Event( _P1 p1, _P2 p2, _P3 p3, _P4 p4 ) : 
		payload_( p1, p2, p3, p4 )
		{}
		template< typename _P1, typename _P2, typename _P3, typename _P4, typename _P5 >
		Event( _P1 p1, _P2 p2, _P3 p3, _P4 p4, _P5 p5 ) : 
		payload_( p1, p2, p3, p4, p5 )
		{}
		template< typename _P1, typename _P2, typename _P3, typename _P4, typename _P5, typename _P6 >
		Event( _P1 p1, _P2 p2, _P3 p3, _P4 p4, _P5 p5, _P6 p6 ) : 
		payload_( p1, p2, p3, p4, p5, p6 )
		{}

		hUint32			EventID() const { return EVENT_ID; }
		hUint32			Size() const { return sizeof( Event< id, _Ty > ); }
		const _Ty*		operator -> () const { return &payload_; }

	private:
		
		_Ty				payload_;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class BaseListener
	{
	public:
		virtual hUint32		EventID() const = 0;
		virtual void		PostEvent( BaseEvent* pEv ) = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	template < typename _Ty >
	class Listener : public BaseListener
	{
	public:
		static const hUint32 EVENT_ID = _Ty::EVENT_ID;
		typedef typename huFunctor< void(*)( hUint32, _Ty& ) >::type Delegate;

		Listener()
		{}

		Listener( Delegate& delegate ) :
			delegateFunction_( delegate )
		{}

		void		SetDelegate( Delegate& delegate ) { delegateFunction_ = delegate; }

		hUint32		EventID() const { return EVENT_ID; }
		void		PostEvent( BaseEvent* pEv )
		{
			delegateFunction_( EVENT_ID, *((_Ty*)(pEv)) );
		}

	private:

		Delegate			delegateFunction_;

	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class Channel
	{
	public:

		static const hUint32 DEFAULT_CHANNEL_SIZE = 8 * 1024;

		Channel( hUint32 channelId );
		~Channel();

		hUint32				ChannelID() const { return channelID_; }
		void				AddListener( BaseListener* listener );
		void				RemoveListener( BaseListener* listener );
		template< hUint32 id, typename _Ty >
		void				PostEvent( const Event< id, _Ty >& pEvent )
		{
			hUint32 newEventBytes_ = eventBytes_ + pEvent.Size();
			while ( newEventBytes_ > postedEvents_.size() )
			{
				//grab another 1k
				postedEvents_.resize( postedEvents_.size() + 1024 );
			}

			hByte* pDst = &postedEvents_[ eventBytes_ ];
			memcpy( pDst, &pEvent, pEvent.Size() );

			eventBytes_ = newEventBytes_;
		}
		void				DispatchEvents();

	private:

		typedef list< BaseListener* > Listeners;

		hUint32					eventBytes_;
		Listeners				listeners_;
		vector< hByte >			postedEvents_;//will prob need to swap this for a lockless queue
		hUint32					channelID_;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class EventManager
	{
	public:
		EventManager();
		~EventManager();

		void				AddChannel( hUint32 channelId );
		void				RemoveChannel( hUint32 channelId );
		void				AddListener( hUint32 channelId, BaseListener* listener );
		void				RemoveListener( hUint32 channelId, BaseListener* listener );
		template< hUint32 id, typename _Ty >
		void				PostEvent( hUint32 channelId, const Event< id, _Ty >& pEvent )
		{
			//for the moment do just a quick linear search
			Channels::const_iterator iend = channels_.end();
			for ( Channels::iterator i = channels_.begin(); i != iend; ++i )
			{
				if ( channelId > (*i)->ChannelID() )
				{
					hcWarningHigh( hTrue, "Channel ID %d not found", channelId );
					return;//not found the channel!
				}
				else if ( channelId == (*i)->ChannelID() )
				{
					(*i)->PostEvent( pEvent );
					return;
				}
			}
		}
		void				DispatchEvents();

	private:

		typedef list< Channel* > Channels;

		Channels				channels_;//< sorted list of channels by id
	};
}
#endif // EVENT_H__