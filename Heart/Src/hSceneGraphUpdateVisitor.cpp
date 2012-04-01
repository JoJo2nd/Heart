/********************************************************************

	filename: 	hSceneGraphUpdateVisitor.cpp	
	
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


namespace Heart
{


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hSceneGraphUpdateVisitor::hSceneGraphUpdateVisitor() :
		pSceneGraph_( NULL )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hSceneGraphUpdateVisitor::~hSceneGraphUpdateVisitor()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneGraphUpdateVisitor::PreVisit( hSceneGraph* pSceneGraph )
	{
		pSceneGraph_ = pSceneGraph;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneGraphUpdateVisitor::Visit( hSceneNodeMesh& visit )
	{
		//visit.GlobalMatrix( *pSceneGraph_->matrixStackTop() );
		//visit.UpdateAABB();
		//visit.MakeClean();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneGraphUpdateVisitor::Visit( hSceneNodeLocator& visit )
	{
		//visit.GlobalMatrix( *pSceneGraph_->matrixStackTop() );
		//visit.UpdateAABB();
		//visit.MakeClean();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneGraphUpdateVisitor::Visit( hSceneNodeCamera& visit )
	{
		//visit.GlobalMatrix( *pSceneGraph_->matrixStackTop() );
		//visit.UpdateAABB();
		//visit.UpdateCamera();
		//visit.MakeClean();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneGraphUpdateVisitor::PostVisit( hSceneGraph* pSceneGraph )
	{
		pSceneGraph_ = NULL;
	}

}

#if 0

void hrSceneGraphUpdateVisitor::Visit( hrSceneGraphNodeCamera& visit )
{
	//hcPrintf( "Update Visitor - Visit Camera %s\n", visit.name() );
	visit.GlobalMartix( *pSceneGraph_->matrixStackTop() );
	//build the view matrix, etc
	visit.UpdateAABB();
	visit.updateCamera();
	visit.MakeClean();
	
}

void hrSceneGraphUpdateVisitor::Visit( hrSceneGraphNodeLight& visit )
{
	//hcPrintf( "Update Visitor - Visit Light %s\n", visit.name() );
	visit.GlobalMartix( *pSceneGraph_->matrixStackTop() );
	visit.UpdateAABB();
	visit.update();
	visit.MakeClean();
	
}

#endif