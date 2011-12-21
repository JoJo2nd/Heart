/********************************************************************
	created:	2009/08/23
	created:	23:8:2009   23:24
	filename: 	huLocklessRingBuffer.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HURINGBUFFER_H__
#define HURINGBUFFER_H__

#include "hTypes.h"
#include "hMutex.h"
#include "hArray.h"
#include "hAtomic.h"

namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	// NOTE: This is designed for one reader thread and one writer thread ////
	//////////////////////////////////////////////////////////////////////////
	template < typename _Ty, hUint32 _SIZE >
	class hLocklessFixedComPipe
	{
	public:

		hLocklessFixedComPipe()
		{
			first_ = 0;
			end_ = 0;
		}

		~hLocklessFixedComPipe()
		{

		}

		void push( const _Ty& a )
		{
			hUint32 end = end_;
			hcAssert( !IsFull() );
			pData_[ end ] = a;
			Heart::hAtomic::LWMemoryBarrier();
			end_ = ( end + 1 ) % _SIZE;
		}

		void pop()
		{
			hUint32 first = first_;
			hcAssert( !IsEmpty() );
			Heart::hAtomic::LWMemoryBarrier();
			first_ = ( first + 1 ) % _SIZE;
		}

		_Ty& peek()
		{
			return pData_[ first_ ];
		}

		hBool IsEmpty()
		{
			return end_ == first_;
		}

		hBool IsFull()
		{
			return (( end_ + 1 ) % _SIZE) == first_;
		}

	private:

		hArray< _Ty, _SIZE >		pData_;
		volatile hUint32			__declspec( align(4) ) first_;//needs 4 byte alignment
		volatile hUint32			__declspec( align(4) ) end_;//needs 4 byte alignment

	};

	//////////////////////////////////////////////////////////////////////////
	// NOTE: This is designed for one reader thread and one writer thread ////
	//////////////////////////////////////////////////////////////////////////
	template< hUint32 MAX_SIZE >
	class hLocklessComPipe
	{
	public:

		hLocklessComPipe() :
		  pFirst_( ringBufferData_ ),
		  pEnd_( ringBufferData_ ),
		  pRingEnd_( NULL ),
		  looped_( hFalse ),
		  pReserve_( NULL ),
		  reservedBytes_( 0 )
		{}
		~hLocklessComPipe() {}

		void			Pop( hUint32 nBytes )
		{
			hcAssert( SpaceUsed() >= nBytes );

			hByte* pRingEnd = pRingEnd_;
			hByte* pFirst = pFirst_;
			hByte* pnFirst;

			pnFirst = pFirst + nBytes;
			if ( pRingEnd && (pFirst == pRingEnd || pnFirst >= pRingEnd) )
			{
				hcAssert( pnFirst == pRingEnd || pFirst == pRingEnd );
				if ( pFirst == pRingEnd )
					pFirst_ = ringBufferData_ + nBytes;
				else
					pFirst_ = ringBufferData_;
				Heart::hAtomic::LWMemoryBarrier();
				pRingEnd_ = NULL;
			}
			else
			{
				// Let users know new data has been popped
				pFirst_ = pnFirst;
			}

			hcAssert( pFirst_ != pRingEnd_ );
		}
		void*			Peek()
		{
			if ( pFirst_ == pRingEnd_ )
			{
				return ringBufferData_;
			}
			return pFirst_;
		}
		void*			Push( void* data, hUint32 nBytes )
		{
			hByte* pEnd = Reserve( nBytes );

			//safely got the space reserved, copy the data
			memcpy( pEnd, data, nBytes );

			ReleaseReserve( pEnd );

			return pEnd;
		}

		void*			Reserve( hUint32 nBytes )
		{
			hcAssert( pReserve_ == NULL );
			hcAssert( (SpaceUsed() - MAX_SIZE) >= nBytes );

			hByte* pEnd;
			hByte* pnEnd;

			pEnd = pEnd_;
			pnEnd = pEnd + nBytes;

			pReserve_ = pEnd;
			reservedBytes_ = nBytes;

			if ( pnEnd >= (ringBufferData_+MAX_SIZE) )
			{
				looped_ = hTrue;
				pReserve_ = ringBufferData_;
			}

			return pReserve_;
		}

		void ReleaseReserve( void* pReserve )
		{
			hcAssert( pReserve_ == pReserve );

			if ( pReserve == ringBufferData_ && looped_ )
			{
				hByte* pend = pEnd_;
				pEnd_ = (hByte*)pReserve + reservedBytes_;
				Heart::hAtomic::LWMemoryBarrier();
				pRingEnd_ = pend;
				looped_ = hFalse;
			}
			else
			{
				// Let users know new data has been pushed
				pEnd_ = (hByte*)pReserve + reservedBytes_;
			}
			pReserve_ = NULL;
		}

		hBool			IsEmpty()
		{
			return SpaceUsed() == 0;
		}

		hBool			HasSpace( hUint32 bytes )
		{
			return (SpaceUsed() - MAX_SIZE) >= bytes;
		}

		hUint32			SpaceUsed()
		{
			hAtomic::LWMemoryBarrier();
			hByte* pringend = pRingEnd_;

			if ( pringend )
			{
				hByte* pfirst = pFirst_;
				hByte* pend = pEnd_;
				hUint32 space = (hUint32)pringend - (hUint32)pfirst;
				space += (hUint32)pend - (hUint32)ringBufferData_;
				return space;
			}
			else
			{
				hByte* pfirst = pFirst_;
				hByte* pend = pEnd_;
				hUint32 space = (hUint32)pend - (hUint32)pfirst;
				return space;
			}
		}

	private:

		__declspec(align(4)) hByte*						 pFirst_;
		hByte*						/*__declspec(align(4)) */pEnd_;
		hByte						/*__declspec(align(4)) */ringBufferData_[ MAX_SIZE ];
		hByte*						pRingEnd_;
		hBool						looped_;

		hByte*						pReserve_;
		hUint32						reservedBytes_;
	};
}

#endif // HURINGBUFFER_H__