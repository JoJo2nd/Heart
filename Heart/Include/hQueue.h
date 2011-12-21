/********************************************************************
	created:	2008/06/08
	created:	8:6:2008   19:33
	filename: 	hcSQueue.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef hcSQueue_h__
#define hcSQueue_h__

#include "hTypes.h"
#include "hArray.h"

namespace Heart
{
	template< class T, hUint32 MaxSize >
	class hQueue
	{
	public:

		hQueue() :
		  Size_( 0 ),
			  Front_( 0 ),
			  Back_( 0 )
		  {
		  }

		  hUint32 size()
		  {
			  return Size_;
		  }

		  void push( const T val )
		  {
			  hcAssert( Size_ + 1 != MaxSize );
			  if ( Size_ + 1 < MaxSize )
			  {
				  QueueEle_[ Back_ ] = val;
				  Back_ = (Back_ + 1) % MaxSize;
				  Size_++;
			  }
			  else
			  {
				  hcBreak;
			  }
		  }

		  T pop()
		  {
			  hcAssert( Size_ != 0 );

			  if ( Size_ >= 0 )
			  {
				  hUint32 n = Front_;
				  Front_ = (Front_ + 1) % MaxSize;
				  Size_--;
				  return QueueEle_[ n ];
			  }

			  return T();
		  }

		  hBool isEmpty()
		  {
			  return Size_ == 0;
		  }

	private:

		hUint32						Size_;
		hUint32						Front_;// next to pop
		hUint32						Back_;// next element should go here
		hArray< T, MaxSize > 		QueueEle_;

	};

	template< class T >
	class huDQueue
	{
	public:
		huDQueue()
		{
			size_ = 0;
			reservedSize_ = 32;
			reservedRegionStart_ = tmalloc< T >( sizeof( T ) * reservedSize_ );
			reservedRegionEnd_ = reservedRegionStart_ + reservedSize_;
		}

		hUint32 size() { return size_; }

		void reserve( hUint32 rsize )
		{
			if ( reservedSize_ < rsize )
			{
				hUint32 frontOS = ( (hUint32)pfront_ - (hUint32)reservedRegionStart_ ) / sizeof( T );
				hUint32 backOS = ( (hUint32)pback_ - (hUint32)reservedRegionStart_ ) / sizeof( T );

				reservedRegionStart_ = realloc( reservedRegionStart_, rsize );
				reservedRegionEnd_ = reservedRegionStart_ + rsize;
				reservedSize_ = rsize;

				//realign front & back
				pfront_ = reservedRegionStart_ + frontOS;
				pback_ = reservedRegionStart_ + backOS;
			}
		}

		void push( const T& val )
		{
			// if run out of space, double it
			if ( size_ == reservedSize_ )
			{
				reserve( reservedSize_ * 2 );
			}

			// copy into queue
			(*pback_) = val;
			++pback_;

			++size_;

			// check for wrap around
			if ( pback_ > reservedRegionEnd_ )
			{
				pback_ = reservedRegionStart_;
			}

		}

		void pop( T& outRes /*return this way because the copy could be heavy*/ )
		{
			hcAssert( Size_ > 0 );

			if ( size_ > 0 )
			{
				//copy out the result
				outRes = (*pfront_);

				--size_;

				++pfront_;

				//check for wrap around
				if ( pfront_ > reservedRegionEnd_ )
				{
					pfront_ = reservedRegionStart_;
				}
			}
		}

		hBool isEmpty(){ return size_ == 0; }

	private:

		hUint32						size_;
		hUint32						reservedSize_;
		T*							pfront_;// next to pop
		T*							pback_;// next element should go here, will loop for speed
		T*							reservedRegionStart_;
		T*							reservedRegionEnd_;

	};
}

#endif // hcSQueue_h__