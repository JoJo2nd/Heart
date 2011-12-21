/********************************************************************
	created:	2009/02/17

	filename: 	FontBuilder.h

	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef __FONTBUILDER_H__
#define __FONTBUILDER_H__

#include "Heart.h"
#include <stdio.h>
#include <vector>
#include <list>
#include "ResourceBuilder.h"
#include "haFont.h"


class FontBuilder : public ResourceBuilder
{
public:

	DEFINE_RESOURCE_BUILDER();

												FontBuilder();
	virtual										~FontBuilder();

	virtual hBool								BuildResource();
	virtual hUint32								GetDataSize();
	virtual hByte*								GetDataPtr();
	virtual hcResourceHeader*					GetResourceHeader();
	virtual ResourceTimeStamp					GetTimeStamp();

private:
	
	hBool										generateFontOutput();
	hBool										readFontHeader();
	hBool										readFontCharacter();

	hUint32										FontSize_;
	std::string									FontFileName_;
	std::string									PNGFileName_;
	haFont										FontResource_;
	haFont*										pFontResource_;
	std::vector< FontCharacter >				FontCharacters_;
	huDMap< FontCharacter >					Characters_;
	FILE*										pFontDefFile_;
	FILE*										pFontPNGFile_;

	hcResourceHeader							MyHeader_;
	hUint32										Size_;
	hByte*										pBuffer_;

};

inline FontBuilder::FontBuilder() :
	ResourceBuilder( classId_ ),
	pFontDefFile_( NULL ),
	pFontPNGFile_( NULL ),
	Size_( 0 ),
	pBuffer_( NULL )
{

}

inline FontBuilder::~FontBuilder()
{
// 	// needs to clean up the font png and .font files [2/17/2009 James]
// 	if ( FontFileName_ != "" )
// 		remove( FontFileName_.c_str() );
// 	if ( PNGFileName_ != "" )
// 		remove( PNGFileName_.c_str() );
}

inline hUint32 FontBuilder::GetDataSize()
{
	return Size_;
}

inline hByte* FontBuilder::GetDataPtr()
{
	return pBuffer_;
}

inline hcResourceHeader* FontBuilder::GetResourceHeader()
{
	return &MyHeader_;
}


#endif //__FONTBUILDER_H__