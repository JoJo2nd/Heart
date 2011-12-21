/********************************************************************
	created:	2010/07/18
	created:	18:7:2010   9:48
	filename: 	DeviceImplFactory.cpp	
	author:		James
	
	purpose:	Win32 version of impl create functions
*********************************************************************/

#include "Common.h"
#include "DeviceImplFactory.h"
#include "DeviceD3D9Renderer.h"
#include "DeviceD3D9Material.h"
#include "DeviceD3D9VtxDecl.h"
#include "DeviceD3D9Texture.h"
#include "DeviceD3D9IndexBuffer.h"
#include "DeviceD3D9VtxBuffer.h"
#include "DeviceInputDeviceManager.h"
#include "DeviceD3D9DepthSurface.h"

namespace Heart
{
namespace Device
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32				ImplFactory::versionInfo_	= 0;
	const hUint32		ImplFactory::rendererMask_	= ( hDIRECT3D9 | hDIRECT3D11 | hOPENGL );
	const hUint32		ImplFactory::osMask_		= ( hWIN32 );

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ImplFactory::SetVersion( hUint32 version )
	{
		//TODO: validate version info passed in
		#pragma message ("TODO: validate version info passed in")
		versionInfo_ = version;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	IRenderer* ImplFactory::CreateRendererImpl()
	{
		switch( versionInfo_ & rendererMask_ )
		{
		case hDIRECT3D9:
			return hNEW ( hRendererHeap ) D3D9Renderer;
		case hDIRECT3D11:
			hcAssertMsg( false, "Not Implemented yet" );
			return NULL;
		case hOPENGL:
			hcAssertMsg( false, "Not Implemented yet" );
			return NULL;
		default:
			hcAssertMsg( false, "Unknown Renderer Implementation" );
			return NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	IMaterial* ImplFactory::CreateMaterialImpl( void* inplace /*=NULL*/)
	{
		switch( versionInfo_ & rendererMask_ )
		{
		case hDIRECT3D9:
			return inplace ? new ( inplace ) D3D9Material() : hNEW ( hRendererHeap ) D3D9Material();
		case hDIRECT3D11:
			hcAssertMsg( false, "Not Implemented yet" );
			return NULL;
		case hOPENGL:
			hcAssertMsg( false, "Not Implemented yet" );
			return NULL;
		default:
			hcAssertMsg( false, "Unknown Renderer Implementation" );
			return NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	IVtxDecl* ImplFactory::CreateVertexDeclarationImpl()
	{
		switch( versionInfo_ & rendererMask_ )
		{
		case hDIRECT3D9:
			return new ( hRendererHeap ) D3D9VtxDecl();
		case hDIRECT3D11:
			hcAssertMsg( false, "Not Implemented yet" );
			return NULL;
		case hOPENGL:
			hcAssertMsg( false, "Not Implemented yet" );
			return NULL;
		default:
			hcAssertMsg( false, "Unknown Renderer Implementation" );
			return NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	ITextureBase* ImplFactory::CreateTexutreBaseImpl( void* inplace /*= NULL*/ )
	{
		switch( versionInfo_ & rendererMask_ )
		{
		case hDIRECT3D9:
			return inplace ? new ( inplace ) D3D9Texture() : hNEW ( hRendererHeap ) D3D9Texture();
		case hDIRECT3D11:
			hcAssertMsg( false, "Not Implemented yet" );
			return NULL;
		case hOPENGL:
			hcAssertMsg( false, "Not Implemented yet" );
			return NULL;
		default:
			hcAssertMsg( false, "Unknown Renderer Implementation" );
			return NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	IIndexBuffer* ImplFactory::CreateIndexBufferImpl( void* inplace /*= NULL*/ )
	{
		switch ( versionInfo_ & rendererMask_ )
		{
		case hDIRECT3D9:
			return inplace ? new( inplace ) hdD3D9IndexBuffer : hNEW ( hRendererHeap ) hdD3D9IndexBuffer();
			break;
		case hDIRECT3D11:
			hcAssertMsg( false, "Not Implemented yet" );
			return NULL;
		case hOPENGL:
			hcAssertMsg( false, "Not Implemented yet" );
			return NULL;
		default:
			hcAssertMsg( false, "Unknown Index Buffer Implementation" );
			return NULL;	
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	IVtxBuffer* ImplFactory::CreateVertexBufferImpl( void* inplace /*= NULL*/ )
	{
		switch ( versionInfo_ & rendererMask_ )
		{
		case hDIRECT3D9:
			return inplace ? new( inplace ) hdD3D9VtxBuffer : hNEW ( hRendererHeap ) hdD3D9VtxBuffer();
			break;
		case hDIRECT3D11:
			hcAssertMsg( false, "Not Implemented yet" );
			return NULL;
		case hOPENGL:
			hcAssertMsg( false, "Not Implemented yet" );
			return NULL;
		default:
			hcAssertMsg( false, "Unknown Vertex Buffer Implementation" );
			return NULL;	
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	IInputDeviceManager* ImplFactory::CreateInputDeviceManager()
	{
		switch( versionInfo_ & osMask_ )
		{
		case hWIN32:
			return hNEW ( hGeneralHeap ) InputDeviceManager();
			break;
		default:
			hcAssertMsg( false, "Unknow Input Device Manager Implementation" );
			return NULL;
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	IDepthSurface* ImplFactory::CreateDepthSurface()
	{
		switch ( versionInfo_ & rendererMask_ )
		{
		case hDIRECT3D9:
			return hNEW ( hRendererHeap ) D3D9DepthSurface();
			break;
		case hDIRECT3D11:
			hcAssertMsg( false, "Not Implemented yet" );
			return NULL;
		case hOPENGL:
			hcAssertMsg( false, "Not Implemented yet" );
			return NULL;
		default:
			hcAssertMsg( false, "Unknown Depth Surface Implementation" );
			return NULL;	
		}
	}

}
}