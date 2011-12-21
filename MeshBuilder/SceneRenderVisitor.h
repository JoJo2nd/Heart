/********************************************************************

	filename: 	SceneRenderVisitor.h	
	
	Copyright (c) 23:1:2011 James Moran
	
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

#ifndef SCENERENDERVISITOR_H__
#define SCENERENDERVISITOR_H__

#include "SceneVisitor.h"

namespace Direct3D
{
	class Device;
}
namespace Data
{
	class Material;
}

namespace HScene
{
	class RenderVisitor : public VisitorBase
	{
	public:
		RenderVisitor( Direct3D::Device* pd3ddevice ) :
			pD3D_( pd3ddevice )
		{

		}
		~RenderVisitor()
		{

		}

		virtual void			Visit( Locator* pVisit );
		virtual void			Visit( MeshCollection* pVisit );
		virtual void			Visit( Camera* pVisit );
		virtual void			Visit( Light* pVisit );

	public:

		Direct3D::Device* pD3D_;
	};
}

#endif // SCENERENDERVISITOR_H__