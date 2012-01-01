/********************************************************************
	created:	2010/07/24
	created:	24:7:2010   23:32
	filename: 	IndexBuffer.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef INDEXBUFFER_H__
#define INDEXBUFFER_H__

#include "hTypes.h"
#include "hResource.h"
#include "HeartSTL.h"
#include "hRendererConstants.h"

namespace Heart
{
	class hRenderer;
namespace Cmd
{
	class FlushIndexBufferData;
}

	class hIndexBuffer : public pimpl< hdIndexBuffer >, public hResourceClassBase
	{
	public:
		hIndexBuffer( hRenderer* prenderer ) 
			: renderer_( prenderer )
			, lockPtr_( NULL )
		{}
		~hIndexBuffer() 
		{
		}

		void			Lock();
		void			Unlock();
		hUint16			IndexCount() const { return nIndices_; };
		void			SetIndex( hUint16 idx, hUint16 val );
		void			SetData( hUint16* data, hUint32 size );

	private:

		friend class hRenderer;
		friend class Cmd::FlushIndexBufferData;
		friend class IndexBufferBuilder;

		void			Release();
		void			FlushIndexData( void* data, hUint32 size );

		hRenderer*		renderer_;
		PrimitiveType	primitiveType_;
		hUint16*		pIndices_;//< only valid on disk based resources
		hUint16			nIndices_;
		hUint16*		lockPtr_;
	};

    struct hIndexBufferMapInfo
    {
        void*           ptr_;
        hUint32         size_;
        hIndexBuffer*   ib_;
    };
}

#endif // INDEXBUFFER_H__