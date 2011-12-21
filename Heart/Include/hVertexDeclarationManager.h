/********************************************************************
	created:	2008/11/23
	created:	23:11:2008   12:01
	filename: 	hrVertexDeclarationManager.h
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HRVERTEXDECLARATIONMANAGER_H__
#define HRVERTEXDECLARATIONMANAGER_H__

#include "hTypes.h"
#include "HeartSTL.h"
#include "hVertexDeclarations.h"


namespace Heart
{
	class hRenderer;

	class hVertexDeclarationManager
	{
	public:
		hVertexDeclarationManager( hRenderer& renderer );
		~hVertexDeclarationManager();

		hVertexDeclaration*								GetVertexDeclartion( hUint32 flags );
		void											Destroy();

	private:

		typedef map< hUint32, hVertexDeclaration* >		VtxDeclMap;

		VtxDeclMap										vertexDeclarations_;
		hRenderer&										renderer_;
	};
}

#endif // HRVERTEXDECLARATIONMANAGER_H__