/********************************************************************

	filename: 	MeshBuilder.h	
	
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

#ifndef INDEXBUFFERBUILDER_H__
#define INDEXBUFFERBUILDER_H__

#include "ResourceBuilder.h"
#include "tinyxml/tinyxml.h"
#include <stack>
#include <vector>

struct PrimitiveGroup;

class IndexBufferBuilder :	public ResourceBuilder,
							public TiXmlVisitor
{
public:
	IndexBufferBuilder();
	~IndexBufferBuilder();

	hBool								BuildResource();
	hUint32								GetDataSize() { return BufferOffset_; }
	hByte*								GetDataPtr() { return pDataBuffer_; }

private:

	virtual bool						VisitExit( const TiXmlElement& element );
	void								WriteResourceData( const PrimitiveGroup* pPrimGroups, hUint16 nGroups );

	TiXmlDocument						meshDef_;
	std::stack< const TiXmlElement* >	elementStack_;
	std::vector< hUint16 >				indices_;

};
#endif // INDEXBUFFERBUILDER_H__
