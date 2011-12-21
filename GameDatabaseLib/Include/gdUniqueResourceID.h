/********************************************************************

	filename: 	gdUniqueResourceID.h	
	
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

#ifndef GDUNIQUERESOURCEID_H__
#define GDUNIQUERESOURCEID_H__

namespace GameData
{
    class gdDLL_EXPORT gdUniqueResourceID
    {
    public:

        static const gdUint     MAX_RESOURCE_NAME_LENGHT = 128;
        static const gdUint     MAX_RESOURCE_PATH_LENGHT = 1024 - MAX_RESOURCE_NAME_LENGHT;

        gdUniqueResourceID()
            : resourceCRC_(0)
        {
        }

        gdUniqueResourceID( const gdUniqueResourceID& rhs );

        ~gdUniqueResourceID()
        {

        }

        gdError                 Initialise( const gdChar* path, const gdChar* name );
        const gdChar*           GetResourcePath() const { return resourcePath_.c_str(); }
        const gdChar*           GetResourceName() const { return resourceName_.c_str(); }
        gdError                 SetResourcePath( const gdChar* newPath );
        gdError                 SetResourceName( const gdChar* newName );
        gdUint32                GetResourceCRCID() const { return resourceCRC_; }

        gdBool                  operator == ( const gdUniqueResourceID& rhs ) const;
        gdBool                  operator == ( const gdUint32& rhs ) const;
        gdUniqueResourceID&     operator = ( const gdUniqueResourceID& rhs );
        gdBool                  operator < ( const gdUniqueResourceID& rhs ) const;
        gdBool                  operator > ( const gdUniqueResourceID& rhs ) const;

        gdDEFINE_SAVE_VERSION( 0 )

    private:

        friend class boost::serialization::access;

        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version )
        {
            arc & BOOST_SERIALIZATION_NVP(resourcePath_);
            arc & BOOST_SERIALIZATION_NVP(resourceName_);
            arc & BOOST_SERIALIZATION_NVP(resourceCRC_);

            BuildResourceCRC();
        }

        void BuildResourceCRC();

        gdString                resourcePath_;
        gdString                resourceName_;
        gdUint32                resourceCRC_;                  
    };
}

gdDECLARE_SAVE_VERSION( GameData::gdUniqueResourceID )

#endif // GDUNIQUERESOURCEID_H__