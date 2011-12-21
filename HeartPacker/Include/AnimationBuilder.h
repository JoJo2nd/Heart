/********************************************************************
	created:	2008/11/30
	created:	30:11:2008   20:35
	filename: 	AnimationBuilder.h
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef ANIMATIONBUILDER_H__
#define ANIMATIONBUILDER_H__

#include "Heart.h"
#include "ResourceBuilder.h"
#include <vector>
#include <list>
#include "FBXImporter.h"

class AnimationBuilder : public ResourceBuilder
{
public:

	DEFINE_RESOURCE_BUILDER();

									AnimationBuilder();
	virtual							~AnimationBuilder();

	virtual	hBool					BuildResource();
	virtual hUint32					GetDataSize();
	virtual hByte*					GetDataPtr();
	virtual hcResourceHeader*		GetResourceHeader();

private:

	FBXImporter						FileImporter_;

	hcResourceHeader				Header_;
	hByte*							pResourceData_;
	hUint32							ResourceSize_;

};

inline AnimationBuilder::AnimationBuilder() :
	ResourceBuilder( classId_ )
{

}

inline AnimationBuilder::~AnimationBuilder()
{

}

inline hUint32 AnimationBuilder::GetDataSize()
{
	return ResourceSize_;
}

inline hByte* AnimationBuilder::GetDataPtr()
{
	return pDataBuffer_;
}

inline hcResourceHeader* AnimationBuilder::GetResourceHeader()
{
	return &Header_;
}

#endif // ANIMATIONBUILDER_H__