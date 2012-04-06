/********************************************************************

	filename: 	hSceneGraphVisitorBase.h	
	
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

#ifndef HRSCENEGRAPHVISITORBASE_H__
#define HRSCENEGRAPHVISITORBASE_H__

namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSceneGraph;
	class hSceneNodeLocator;
	class hSceneNodeLight;
	class hSceneNodeMesh;
	class hSceneNodeCamera;
 
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSceneGraphVisitorBase
	{
	public:

		hSceneGraphVisitorBase() 
			: abortChild_( hFalse )
		{

		}

		virtual void	PreVisit( hSceneGraph* /*pSceneGraph*/ ) {}
		virtual void	PostVisit( hSceneGraph* /*pSceneGraph*/ ) {}
		void			AbortChildVisit( hBool v ) { abortChild_ = v; }
		hBool			AbortChildVisit() const { return abortChild_; }
		virtual hBool 	IsEngineBase() { return hFalse; }

	private:

		hBool			abortChild_;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSceneGraphVisitorEngine : public hSceneGraphVisitorBase
	{
	public:

		hSceneGraphVisitorEngine()
		{

		}

		virtual void	Visit( hSceneNodeLocator& visit ) {};
		virtual void	Visit( hSceneNodeMesh& visit ) {};
		virtual void	Visit( hSceneNodeCamera& visit ) {};
		virtual void	Visit( hSceneNodeLight& visit ) {};

	private:

	};

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define DEFINE_VISITABLE( nodeclass ) \
	virtual void Accept( Heart::hSceneGraphVisitorBase* guest ) { return AcceptImpl(*this, guest); } \
	void AcceptImpl( nodeclass& visited, Heart::hSceneGraphVisitorBase* guest );

#define ACCEPT_VISITOR( nodeclass, visitorbase ) \
	void nodeclass::AcceptImpl( nodeclass& visited, Heart::hSceneGraphVisitorBase* guest ) \
	{ \
		if ( guest->IsEngineBase() == hFalse )\
		{\
			visitorbase* pvis = static_cast< visitorbase* >( guest ); \
			pvis->Visit( visited ); \
		}\
	} 

#define ENGINE_ACCEPT_VISITOR( nodeclass, visitorbase ) \
	void nodeclass::AcceptImpl( nodeclass& visited, Heart::hSceneGraphVisitorBase* guest ) \
	{ \
		visitorbase* pvis = static_cast< visitorbase* >( guest ); \
		pvis->Visit( visited );\
	} 

#define ENGINE_DEFINE_VISITOR() \
	virtual hBool 	IsEngineBase() { return hTrue; }

#endif // HRSCENEGRAPHVISITORBASE_H__