/********************************************************************

	filename: 	SceneVisitor.h	
	
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

#ifndef SCENEVISITOR_H__
#define SCENEVISITOR_H__

namespace HScene
{
	class Locator;
	class MeshCollection;
	class Camera;
	class Light;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class VisitorBase
	{
	public:
		virtual ~VisitorBase() {}

		virtual void			Visit( Locator* pVisit ) = 0;
		virtual void			Visit( MeshCollection* pVisit ) = 0;
		virtual void			Visit( Camera* pVisit ) = 0;
		virtual void			Visit( Light* pVisit ) = 0;
	private:
	};
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define DEFINE_VISITABLE( nodeclass ) \
	virtual void Accept( HScene::VisitorBase* guest ) { return AcceptImpl(*this, guest); } \
	void AcceptImpl( nodeclass& visited, HScene::VisitorBase* guest );

#define ACCEPT_VISITOR( nodeclass ) \
	void nodeclass::AcceptImpl( nodeclass& visited, HScene::VisitorBase* guest ) \
	{ \
		guest->Visit( &visited ); \
	}

#endif // SCENEVISITOR_H__