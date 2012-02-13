/********************************************************************

	filename: 	SceneBundle.h	
	
	Copyright (c) 17:3:2011 James Moran
	
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

#ifndef SCENEBUNDLE_H__
#define SCENEBUNDLE_H__

#include "hSceneNodeBase.h"

namespace Heart
{
	class hMesh;
	class hMaterial;
	class hSceneNodeBase;

	class hSceneBundle : public hResourceClassBase
	{
	public:
		hSceneBundle();
		~hSceneBundle();

		struct ChildLink
		{
			hUint32		nChildren_;
			hUint32*	pChildren_;
			hUint32		nMeshes_;
			hUint32*	pMeshIdx_;
		};

		//hResourceHandle< hSceneNodeBase >*	GetRootNode() const { return pNodes_; }

#ifndef HEART_RESOURCE_BUILDER
	private:	
#endif

		friend class SceneBuilder;
		friend class hSceneGraph;

		//hResourceHandle< hSceneNodeBase >	rootNode_;
		hUint32											nNodes_;
		//hResourceHandle< hSceneNodeBase >*	pNodes_;
		ChildLink*										pChildLinks_;//size = nNodes
		hUint32											nMeshes_;
		//hResourceHandle< hMesh >*			pMeshes_;
	};
}

#endif // SCENEBUNDLE_H__