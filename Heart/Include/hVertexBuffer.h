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
			, lockPtr_( NULL )
		{}
		~hVertexBuffer() 
		{
			if ( IsDiskResource() )
			{
				SetImpl( NULL );
			}
		}

		void				SetVertexDeclarartion( hVertexDeclaration* pVtxDecl ) { pVtxDecl_ = pVtxDecl; }
		hVertexDeclaration*	GetVertexDeclaration() const { return pVtxDecl_; }
		void				Lock();
		void				Unlock();
		hUint32				VertexCount() const { return vtxCount_; }
		template< typename _Ty >
		void				SetElement( hUint32 idx, VertexElement element, const _Ty& val )
		{
			hcAssertMsg( pVtxDecl_, "No vertex declaration assigned to vertex buffer" );
			hUint32 offset = pVtxDecl_->GetElementOffset( element );
			SetData( idx, pVtxDecl_->Stride(), offset, (void*)&val, sizeof( _Ty ) );
		}

	private:

		friend class hRenderer;
		friend class Cmd::FlushVertexBufferData;
		friend class VertexBufferBuilder;

		void					Release();
		void					SetData( hUint32 idx, hUint32 stride, hUint32 offset, void* pData, hUint32 size );
		void					FlushVertexData( void* dataPtr, hUint32 size );

		hVertexDeclaration*		pVtxDecl_;
		hRenderer*				renderer_;
		hUint32					vtxCount_;
		hUint32					vtxBufferSize_;
		void*					pVtxBuffer_;
		hByte*					lockPtr_;

	};
}

#endif // VERTEXBUFFER_H__