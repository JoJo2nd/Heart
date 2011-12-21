/********************************************************************

	filename: 	MeshCollectionBuilder.h	
	
	Copyright (c) 27:3:2011 James Moran
	
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

#ifndef MESHCOLLECTIONBUILDER_H__
#define MESHCOLLECTIONBUILDER_H__

#include <stdio.h>
#include "ResourceBuilder.h"
#include "RendererConstants.h"
#include "Resource.h"
#include "SceneNodeBase.h"
#include "SceneNodeMesh.h"


class MeshCollectionBuilder : public ResourceBuilder
{
public:
	MeshCollectionBuilder();
	~MeshCollectionBuilder();

	hBool								BuildResource();
	hUint32								GetDataSize() { return BufferOffset_; }
	hByte*								GetDataPtr() { return pDataBuffer_; }

private:

	Heart::Scene::SceneNodeMesh			meshNode_;
};


#endif // MESHCOLLECTIONBUILDER_H__