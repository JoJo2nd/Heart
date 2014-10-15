/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef hcSQueue_h__
#define hcSQueue_h__

namespace Heart
{
	template< class T, hUint32 MaxSize >
	class  hQueue
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

}

#endif // hcSQueue_h__