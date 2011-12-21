/********************************************************************
	created:	2008/09/08
	created:	8:9:2008   20:22
	filename: 	EffectBuilder.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef EffectBuilder_h__
#define EffectBuilder_h__

#include "ResourceBuilder.h"
#include "hResources.h"
#include <list>
#include <string>

class EffectBuilder : public ResourceBuilder
{
public:

	DEFINE_RESOURCE_BUILDER();

										EffectBuilder();
	virtual								~EffectBuilder();

	virtual hBool						BuildResource();
	virtual hUint32						GetDataSize();
	virtual hByte*						GetDataPtr();
	virtual hcResourceHeader*			GetResourceHeader();

private:

	typedef std::list< std::string >	TechList;
	typedef TechList::iterator			TechListItr;

	hcResourceHeader					MyHeader_;
	hByte*								CompiledEffect_;
	hUint32								CompiledEffectSize_;
	hByte*								DebugCompiledEffect_;
	hUint32								DebugCompiledEffectSize_;
	haEffect*							pResource_;
	hByte*								pFileData_;
	hUint32								FileDataSize_;
	TechList							TechniqueList_;

};

inline EffectBuilder::EffectBuilder() :
	ResourceBuilder( classId_ )
{

	CompiledEffect_ = NULL;
	DebugCompiledEffect_ = NULL;
	pFileData_ = NULL;

	FileDataSize_ = 0;

}

inline EffectBuilder::~EffectBuilder()
{

}

inline hUint32 EffectBuilder::GetDataSize()
{
	return FileDataSize_;
}

inline hByte* EffectBuilder::GetDataPtr()
{
	return pFileData_;
}

inline hcResourceHeader* EffectBuilder::GetResourceHeader()
{
	return &MyHeader_;
}

#endif // EffectBuilder_h__