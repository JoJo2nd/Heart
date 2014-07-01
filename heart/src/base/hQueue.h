/********************************************************************

	filename: 	hQueue.h	
	
	Copyright (c) 26:4:2012 James Moran
	
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