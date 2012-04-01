/********************************************************************

	filename: 	hSceneGraphUpdateVisitor.h	
	
	Copyright (c) 1:4:2012 James Moran
	
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

#ifndef HRSCENEGRAPHUPDATEVISITOR_H__
#define HRSCENEGRAPHUPDATEVISITOR_H__

namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSceneGraphUpdateVisitor : public hSceneGraphVisitorEngine
	{
	public:
		
		hSceneGraphUpdateVisitor();
		virtual ~hSceneGraphUpdateVisitor();

		ENGINE_DEFINE_VISITOR();

		virtual void PreVisit( hSceneGraph* pSceneGraph );
		virtual void PostVisit( hSceneGraph*	pSceneGraph );
		virtual void Visit( hSceneNodeMesh& visit );
		virtual void Visit( hSceneNodeLocator& visit );
		virtual void Visit( hSceneNodeCamera& visit );
		
	private:

		hSceneGraph*			pSceneGraph_;
	};

}

#endif // HRSCENEGRAPHUPDATEVISITOR_H__