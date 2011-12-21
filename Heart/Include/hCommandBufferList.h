/********************************************************************
	created:	2010/10/02
	created:	2:10:2010   21:13
	filename: 	CommandBufferList.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef DISPLAYLIST_H__
#define DISPLAYLIST_H__

#include "hTypes.h"

namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hRenderer;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hCommandBufferList
	{
	public:
		hCommandBufferList() :
			size_( 0 )
			,pCommands_( NULL )
			,allocSize_( 0 )
		{

		}
		~hCommandBufferList();

		void													Clear()
		{
			CleanUpCommandList();
			size_ = 0;
			pCommandsEnd_ = pCommands_;
		}

		template< typename _Ty >
		_Ty*													NewRenderCommand() 
		{
			AllocCommandSpace( sizeof( _Ty ) );
			hByte* pPtr = pCommandsEnd_;
			pCommandsEnd_ += sizeof( _Ty );
			size_ += sizeof( _Ty );
			return new ( pPtr ) _Ty;
		}
		template< typename _Ty, typename _P1 >
		_Ty*													NewRenderCommand( _P1 p1 ) 
		{
			AllocCommandSpace( sizeof( _Ty ) );
			hByte* pPtr = pCommandsEnd_;
			pCommandsEnd_ += sizeof( _Ty );
			size_ += sizeof( _Ty );
			return new ( pPtr ) _Ty( p1 );
		}
		template< typename _Ty, typename _P1, typename _P2 >
		_Ty*													NewRenderCommand( _P1 p1, _P2 p2 )
		{
			AllocCommandSpace( sizeof( _Ty ) );
			hByte* pPtr = pCommandsEnd_;
			pCommandsEnd_ += sizeof( _Ty );
			size_ += sizeof( _Ty );
			return new ( pPtr ) _Ty( p1, p2 );
		}
		template< typename _Ty, typename _P1, typename _P2, typename _P3 >
		_Ty*													NewRenderCommand( _P1 p1, _P2 p2, _P3 p3 )
		{
			AllocCommandSpace( sizeof( _Ty ) );
			hByte* pPtr = pCommandsEnd_;
			pCommandsEnd_ += sizeof( _Ty );
			size_ += sizeof( _Ty );
			return new ( pPtr ) _Ty( p1, p2, p3 );
		}
		template< typename _Ty, typename _P1, typename _P2, typename _P3, typename _P4 >
		_Ty*													NewRenderCommand( _P1 p1, _P2 p2, _P3 p3, _P4 p4 )
		{
			AllocCommandSpace( sizeof( _Ty ) );
			hByte* pPtr = pCommandsEnd_;
			pCommandsEnd_ += sizeof( _Ty );
			size_ += sizeof( _Ty );
			return new ( pPtr ) _Ty( p1, p2, p3, p4 );
		}
		template< typename _Ty, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5 >
		_Ty*													NewRenderCommand( _P1 p1, _P2 p2, _P3 p3, _P4 p4, _P5 p5 )
		{
			AllocCommandSpace( sizeof( _Ty ) );
			hByte* pPtr = pCommandsEnd_;
			pCommandsEnd_ += sizeof( _Ty );
			size_ += sizeof( _Ty );
			return new ( pPtr ) _Ty( p1, p2, p3, p4, p5 );
		}
		template< typename _Ty, typename _P1, typename _P2, typename _P3, typename _P4, typename _P5, typename _P6 >
		_Ty*													NewRenderCommand( _P1 p1, _P2 p2, _P3 p3, _P4 p4, _P5 p5, _P6 p6 )
		{
			AllocCommandSpace( sizeof( _Ty ) );
			hByte* pPtr = pCommandsEnd_;
			pCommandsEnd_ += sizeof( _Ty );
			size_ += sizeof( _Ty );
			return new ( pPtr ) _Ty( p1, p2, p3, p4, p5, p6 );
		}

		void			DoCommandList( hRenderer* pRenderer );

	private:

		friend class hRenderer;

		static const hUint32 MIN_ALLOC_SIZE = 256;

		void			AllocCommandSpace( hUint32 needed );
		void			CleanUpCommandList();

		hUint32			size_;
		hUint32			allocSize_;
		hByte*			pCommands_;
		hByte*			pCommandsEnd_;
	};
}

#endif // DISPLAYLIST_H__