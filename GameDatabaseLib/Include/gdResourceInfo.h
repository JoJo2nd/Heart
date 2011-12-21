/********************************************************************

	filename: 	gdResourceInfo.h	
	
	Copyright (c) 25:10:2011 James Moran
	
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
#ifndef GDRESOURCEINFO_H__
#define GDRESOURCEINFO_H__

namespace GameData
{
    class gdGameDatabaseObject;

    typedef std::vector< gdParameterPair >  gdParametersArrayType;

    class gdDLL_EXPORT gdResourceInfo
    {
    public:
        gdResourceInfo();
        ~gdResourceInfo();

        const gdUniqueResourceID* GetResourceID() const;
        void SetResourceID( const gdUniqueResourceID& id );
        void SetInputFilePath( const gdChar* inputPath );
        const gdChar* GetInputFilePath() const;
        gdParameterValue* GetParameter( const gdChar* name );
        gdError SetResourceType( const gdChar* typeName, gdGameDatabaseObject* database );
        const gdChar* GetResourceTypeName() const;
        gdError ApplyResourceParametersFromResourceType( const gdPlugInInformation& resPlugIn );
        gdParametersArrayType* GetResourceParameterArray();
        const gdParametersArrayType* GetResourceParameterArray() const;
        const gdUint32 GetResourceParameterCRC() const;

        gdDEFINE_SAVE_VERSION( 1 )

    private:
        // Hide stupid ms warning C4251
        // (XXX has no DLL interface warning. Things that are private 
        // in the class are not inline to avoid exposing them to outside libs.
        // All work with the private members is hidden in the dll implementation.)
#pragma warning( push )
#pragma warning( disable: 4251 )        
        friend class boost::serialization::access;

        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version )
        {
            switch ( version )
            {
            case 1:
                arc & BOOST_SERIALIZATION_NVP(resourceType_);
                arc & BOOST_SERIALIZATION_NVP(parameters_);
            case 0:
                arc & BOOST_SERIALIZATION_NVP(resID_);
                arc & BOOST_SERIALIZATION_NVP(inputPath_);
            default:
                break;
            }
        }

        gdParameterPair*      GetParameterPair( const gdChar* name );

        gdPREVENT_COPY( gdResourceInfo );

        gdUniqueResourceID          resID_;
        gdString                    inputPath_;
        gdString                    resourceType_;
        gdParametersArrayType       parameters_;
        //Stop Hiding warning C4251
#pragma warning ( pop )
    };
}

gdDECLARE_SAVE_VERSION( GameData::gdResourceInfo )

#endif // GDRESOURCEINFO_H__