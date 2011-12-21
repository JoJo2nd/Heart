/********************************************************************
	created:	2009/02/11

	filename: 	FileImporter.h

	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef __FILEIMPORTER_H__
#define __FILEIMPORTER_H__

#include "hcTypes.h"
#include <string>

class PackerMesh;
class PackerSkeleton;

class FileImporter
{
public:

	virtual hBool						open( const hChar* name ) = 0;
	virtual hBool						readAllData() = 0;
	virtual hBool						exportToPackerMesh( PackerMesh* pMesh ) = 0;
	//hBool								exportToPackerAnimation( PackerAnimation* pAnim ) = 0;
	virtual void						close() = 0;
	virtual std::string					getErrorString() = 0;
	virtual hUint32						nTextures() const = 0;
	virtual const std::string&			getTexture( hUint32 i ) = 0;
	virtual hBool						hasBones() const = 0;

private:

};

#endif //__FILEIMPORTER_H__
