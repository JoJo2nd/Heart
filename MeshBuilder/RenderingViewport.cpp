/********************************************************************

	filename: 	RenderingViewport.cpp	
	
	Copyright (c) 16:1:2011 James Moran
	
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

#include "stdafx.h"
#include "RenderingViewport.h"
#include "SceneRenderVisitor.h"
#include "SceneGraph.h"

namespace Rendering
{


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool Viewport::Initialise( HWND handle )
	{
		hWnd_ = handle;
		if ( !Direct3D::CreateD3DDevice( hWnd_, &device_ ) )
		{
			return false;
		}
		pScene_ = NULL;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Viewport::Resize( u32 width, u32 height )
	{
		width_ = width;
		height_ = height;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Viewport::Reset()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Viewport::Finalise()
	{
		Direct3D::DestroyD3DDevice( &device_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void RenderViewport( Viewport* pViewport )
	{
		if ( pViewport->Enabled() )
		{
			pViewport->pDevice()->BeginRender( pViewport->GetWidth(), pViewport->GetHeight() );
			pViewport->pDevice()->ClearTarget( D3DCOLOR_ARGB( 255, 32, 32, 32 ), 1.0f );

			//TODO: stuff
			if ( pViewport->pScene() )
			{
				HScene::RenderVisitor visitor( pViewport->pDevice() );
				HScene::VisitScene( pViewport->pScene(), &visitor );
			}

			pViewport->pDevice()->EndRender();
		}
	}

}