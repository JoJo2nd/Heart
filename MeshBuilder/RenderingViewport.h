/********************************************************************

	filename: 	RenderViewport.h	
	
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

#ifndef RENDERVIEWPORT_H__
#define RENDERVIEWPORT_H__

#include "stdafx.h"

namespace HScene
{
	class SceneGraph;
}

namespace Rendering
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class Viewport
	{
	public:

		Viewport() :
			hWnd_( NULL )
			,enabled_( true )
			,pScene_( NULL )
			,width_( 100 )
			,height_( 100 )
		{

		}
		~Viewport()
		{

		}

		bool 				Initialise( HWND handle );
		void 				Resize( u32 width, u32 height );
		void 				Reset();
		void 				Finalise();
		u32					GetWidth() const { return width_; }
		u32					GetHeight() const { return height_; }
		HWND				hWnd() const { return hWnd_; }
		Direct3D::Device*	pDevice() { return &device_; }
		void				Enabled( bool val ) { enabled_ = val; }
		bool				Enabled() const { return enabled_; }
		HScene::SceneGraph*	pScene() { return pScene_; }
		void				pScene( HScene::SceneGraph* pScene ) { pScene_ = pScene; }

	private:

		Direct3D::Device	device_;
		HWND				hWnd_;
		bool				enabled_;
		HScene::SceneGraph*	pScene_;
		u32					width_;
		u32					height_;
	};

	void RenderViewport( Viewport* pViewport );
}

#endif // RENDERVIEWPORT_H__