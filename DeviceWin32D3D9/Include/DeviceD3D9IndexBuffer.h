/********************************************************************
	created:	2010/07/24
	created:	24:7:2010   22:59
	filename: 	DeviceD3D9IndexBuffer.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef DEVICED3D9INDEXBUFFER_H__
#define DEVICED3D9INDEXBUFFER_H__

namespace Heart
{
	class hdD3D9IndexBuffer
	{
	public:
		hdD3D9IndexBuffer() 
			: map_( NULL )
			, buffer_( NULL )
		{}
		virtual ~hdD3D9IndexBuffer() {}

		hBool		Bound() const { return buffer_ != NULL; }
		hUint16		IndexCount() const { return nIndices_; }
		void*		Map() 
		{ 
			buffer_->Lock( 0, 0, &map_, 0/*D3DLOCK_DISCARD*/ ); 
			return map_;  
		}
		void		Unmap( void* ptr ) 
		{
			hcAssert( map_ == ptr );
			buffer_->Unlock();
			map_ = NULL;
		}

	private:

		friend class hdD3D9Renderer;
		friend class IndexBufferBuilder;

		LPDIRECT3DINDEXBUFFER9		buffer_;
		hUint16						nIndices_;
		D3DPRIMITIVETYPE			primType_;
		hUint32						mode_;
		void*						map_;
	};
}

#endif // DEVICED3D9INDEXBUFFER_H__
