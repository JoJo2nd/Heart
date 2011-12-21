/********************************************************************

	filename: 	gdUniqueResourceID.cpp	
	
	Copyright (c) 13:10:2011 James Moran
	
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

#include "gdUniqueResourceID.h"
#include "cryptoCRC32.h"

namespace GameData
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdUniqueResourceID::Initialise( const gdChar* path, const gdChar* name )
    {
        if ( strlen(path) >= MAX_RESOURCE_PATH_LENGHT ||
             strlen(name) >= MAX_RESOURCE_NAME_LENGHT )
            return gdERROR_PATH_TOO_LONG;

        //strcpy_s( resourcePath_, MAX_RESOURCE_PATH_LENGHT, path );
        //strcpy_s( resourceName_, MAX_RESOURCE_NAME_LENGHT, name );
        resourcePath_ = path;
        resourceName_ = name;

        BuildResourceCRC();

        if ( !resourceCRC_ )
            return gdERROR_INVALID_CRC;

        return gdERROR_OK;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void gdUniqueResourceID::BuildResourceCRC()
    {
        cyStartCRC32( &resourceCRC_, (gdByte*)resourcePath_.c_str(), resourcePath_.length() );
        cyContinueCRC32( &resourceCRC_, (gdByte*)resourceName_.c_str(), resourceName_.length() );
        resourceCRC_ = cyFinaliseCRC32( &resourceCRC_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdUniqueResourceID::SetResourcePath( const gdChar* newPath )
    {
        resourcePath_ = newPath;
        std::transform( resourcePath_.begin(), resourcePath_.end(), resourcePath_.begin(), toupper );
        BuildResourceCRC();

        return resourceCRC_ ? gdERROR_OK : gdERROR_INVALID_CRC;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdUniqueResourceID::SetResourceName( const gdChar* newName )
    {
        resourceName_ = newName;
        std::transform( resourceName_.begin(), resourceName_.end(), resourceName_.begin(), toupper );
        BuildResourceCRC();

        return resourceCRC_ ? gdERROR_OK : gdERROR_INVALID_CRC;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdBool gdUniqueResourceID::operator==( const gdUniqueResourceID& rhs ) const
    {
        return GetResourceCRCID() == rhs.GetResourceCRCID();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdBool gdUniqueResourceID::operator==( const gdUint32& rhs ) const
    {
        return GetResourceCRCID() == rhs;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdBool gdUniqueResourceID::operator<( const gdUniqueResourceID& rhs ) const
    {
        return GetResourceCRCID() < rhs.GetResourceCRCID();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdBool gdUniqueResourceID::operator>( const gdUniqueResourceID& rhs ) const
    {
        return !(*this < rhs);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdUniqueResourceID::gdUniqueResourceID( const gdUniqueResourceID& rhs )
    {
        resourcePath_ = rhs.resourcePath_;
        resourceName_ = rhs.resourceName_;
        resourceCRC_  = rhs.resourceCRC_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdUniqueResourceID& gdUniqueResourceID::operator=( const gdUniqueResourceID& rhs )
    {
        resourcePath_ = rhs.resourcePath_;
        resourceName_ = rhs.resourceName_;
        resourceCRC_  = rhs.resourceCRC_;

        return *this;
    }

}