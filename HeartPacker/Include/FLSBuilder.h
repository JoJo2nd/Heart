/********************************************************************
	created:	2009/10/04
	created:	4:10:2009   13:28
	filename: 	FLSBuilder.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef FLSBUILDER_H__
#define FLSBUILDER_H__

#include "Heart.h"
#include <stdio.h>
#include <vector>
#include <list>
#include "ResourceBuilder.h"

struct FLSCommand
{
	std::string						symbol_;	//< any value other than "" will strip the texture
	hUint32							textureID_;	//< hErrorCode if same as Previous command or stripped by symbol name
	hFloat							colour_[ 4 ];// RGBA 0.0f..1.0f
	Vec2							coords_[ 4 ];// tl, tr, bl, br
};

struct FLSFrame
{
	hUint32							frameTime_;
	std::list< FLSCommand >			commands_;
};

struct FLSLabel
{
	hUint32							timeMs_;
	std::string						name_;
};

class FLSBuilder : public ResourceBuilder
{
public:

	DEFINE_RESOURCE_BUILDER();

	FLSBuilder() :
		ResourceBuilder( classId_ )
	{

	}
	virtual							~FLSBuilder()
	{

	}

	virtual hBool					BuildResource();
	virtual hUint32					GetDataSize() { return BufferSize_; }
	virtual hByte*					GetDataPtr() { return pDataBuffer_; }
	virtual hcResourceHeader*		GetResourceHeader() { return &header_; }

private:

	hBool							parseFile();
	hBool							parseHeader( FILE* pFile );
	hBool							parseLabels( FILE* pFile );
	/*
		return -1 on error
		0 to continue
		1 on end
	*/
	hUint32							parseFrame( FILE* pFile );
	void							gatherStats();
	void							calcSpaceNeeded( hUint32& totalSize, hUint32& commandBufferSize );
	void							writeData();
	hUint32							getTextureID( hUint32 texid ) 
	{
		hUint32 count = 0;
		std::list< hUint32 >::const_iterator tidend = usedTextureIds_.end();
		for ( std::list< hUint32 >::iterator iTex = usedTextureIds_.begin(); iTex != tidend; ++iTex, ++count )
		{
			if ( *iTex == texid )
				return count;
		}	
		
		return hErrorCode;
	}
	hUint32							getSymbolID( std::string& symbol ) 
	{
		hUint32 count = 0;
		std::list< std::string >::const_iterator tidend = symbols_.end();
		for ( std::list< std::string >::iterator iSym = symbols_.begin(); iSym != tidend; ++iSym, ++count )
		{
			if ( *iSym == symbol )
				return count;
		}	

		return hErrorCode;
	}


	hUint32							width_;
	hUint32							height_;
	hUint32							nTextures_;
	hUint32							nSymbols_;
	hUint32							nLables_;
	hUint32							cmdBufferSize_;
	std::list< FLSLabel >			labels_;
	std::list< std::string >		symbols_;
	std::list< FLSFrame >			frames_;
	std::list< hUint32 >			usedTextureIds_;
	hcResourceHeader				header_;
};

#endif // FLSBUILDER_H__