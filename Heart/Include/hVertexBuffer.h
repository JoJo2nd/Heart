/********************************************************************
	created:	2010/07/25
	created:	25:7:2010   0:40
	filename: 	VertexBuffer.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef VERTEXBUFFER_H__
#define VERTEXBUFFER_H__

#include "hVertexDeclarations.h"
#include "hResource.h"

namespace Heart
{
	class hRenderer;
namespace Cmd
{
	class FlushVertexBufferData;
}

	class hVertexBuffer : public pimpl< hdVtxBuffer >, public hResourceClassBase
	{
	public:

		hVertexBuffer( hRenderer* prenderer )
			: renderer_( prenderer )
		{}
		~hVertexBuffer() 
		{
		}

// 		void				SetVertexDeclarartion( hVertexDeclaration* pVtxDecl ) { pVtxDecl_ = pVtxDecl; }
// 		hVertexDeclaration*	GetVertexDeclaration() const { return pVtxDecl_; }
		void				Lock();
		void				Unlock();
        hUint32             GetStride() const { return stride_; }
		hUint32				VertexCount() const { return vtxCount_; }
		template< typename _Ty >
		void				SetElement( hUint32 idx, VertexElement element, const _Ty& val )
		{
		}

	private:

		friend class hRenderer;
		friend class Cmd::FlushVertexBufferData;
		friend class VertexBufferBuilder;

		void					Release();
		void					SetData( hUint32 idx, hUint32 stride, hUint32 offset, void* pData, hUint32 size );
		void					FlushVertexData( void* dataPtr, hUint32 size );

		hRenderer*				renderer_;
        hUint32                 stride_;
		hUint32					vtxCount_;
		hUint32					vtxBufferSize_;
		void*					pVtxBuffer_;
		hByte*					lockPtr_;
	};

    struct hVertexBufferMapInfo
    {
        void*           ptr_;
        hUint32         size_;
        hVertexBuffer*  vb_;
    };
}

#endif // VERTEXBUFFER_H__