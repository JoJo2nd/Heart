
/********************************************************************
	created:	2010/07/25
	created:	25:7:2010   10:56
	filename: 	DeviceD3D9VtxBuffer.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef DEVICED3D9VTXBUFFER_H__
#define DEVICED3D9VTXBUFFER_H__


namespace Heart
{
	class hdD3D9VtxBuffer
	{
	public:
		hdD3D9VtxBuffer()
			: pBuffer_( NULL )
		{}
		virtual ~hdD3D9VtxBuffer() 
		{
		}

		hBool		Bound() const { return pBuffer_ != NULL; }
		hUint32		VertexCount() const { return nVertex_; };
		hUint32		MaxVertexCount() const { return maxVertex_; };

		void*		Map() { return NULL; }
		void		Unmap( void* ptr ) {}

	private:

		friend class hdD3D9Renderer;
		friend class VertexBufferBuilder;

		hUint32						maxVertex_;
		hUint32						nVertex_;
		LPDIRECT3DVERTEXBUFFER9		pBuffer_;
		hUint32						mode_;
	};
}

#endif // DEVICED3D9VTXBUFFER_H__