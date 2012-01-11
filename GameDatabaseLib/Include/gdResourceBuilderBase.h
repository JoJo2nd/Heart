/********************************************************************

	filename: 	gdResourceBuilderBase.h	
	
	Copyright (c) 7:11:2011 James Moran
	
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

#ifndef GDRESOURCEBUILDERBASE_H__
#define GDRESOURCEBUILDERBASE_H__

namespace GameData
{
    class gdDLL_EXPORT gdTimestampInfo
    {
    public:
        gdTimestampInfo();
        gdTimestampInfo( const gdTimestampInfo& rhs );
        ~gdTimestampInfo();

        void     AddFileTimestamp( const gdFileTimestamp& timestamp );
        gdBool   FileTimestampsValid() const;
        void     SetDllTimestamp( gdTime_t timestamp );
        gdTime_t GetDllTimestamp() const;
        void     SetParamCRC( gdUint32 crc );
        gdUint32 GetParamCRC() const;

        gdBool operator == ( const gdTimestampInfo& rhs ) const;
        gdBool operator != ( const gdTimestampInfo& rhs ) const;
        gdTimestampInfo& operator = ( const gdTimestampInfo& rhs );

        gdDEFINE_SAVE_VERSION( 0 )

    private:
        // Hide stupid ms warning C4251 (XXX has no DLL interface warning)
#pragma warning( push )
#pragma warning( disable: 4251 )
        typedef std::vector< gdFileTimestamp > FileTimestampArrayType;

        friend class boost::serialization::access;

        template< typename _Ty >
        void serialize( _Ty& arc, const unsigned int version )
        {
            switch ( version )
            {
            case 0:
                arc & BOOST_SERIALIZATION_NVP( dllTimestamp_ );
                arc & BOOST_SERIALIZATION_NVP( parameterCRC_ );
                arc & BOOST_SERIALIZATION_NVP( fileTimestamps_ );
            default:
                break;
            }
        }

        gdTime_t                dllTimestamp_;
        gdUint32                parameterCRC_;
        FileTimestampArrayType  fileTimestamps_;
        //Stop Hiding warning C4251
#pragma warning ( pop )
    };

    struct gdResourceBuilderConstructionInfo;
    typedef std::map< hUint32, gdResourceInfo* > gdDependencyArrayType;

    class gdDLL_EXPORT gdResourceBuilderBase
    {
    public:
        gdResourceBuilderBase( const gdResourceBuilderConstructionInfo& );
        virtual ~gdResourceBuilderBase();

        gdFileHandle* OpenFile( const gdChar* path ); //open mem mapped file, read only
        void CloseFile( gdFileHandle* handle ); //cloase mem mapped file.
        gdFileHandle* GetInputFile();
        gdFileHandle* GetOutputFile();
        Heart::hSerialiser* GetSerilaiseObject(); //get object which does our serilaisation 
        const gdTimestampInfo& GetTimestampInfo() const; //includes: input params crc, opened files timestamps & sizes & dll timestamps
        const gdUniqueResourceID& GetResourceURI() const; //get gdResourceUniqueID struct for resource
        const gdParameterValue& GetParameter( const gdChar* param ) const;
        void ThrowFatalError( const gdChar* msg, ... ) const;
        const gdChar* GetErrorMessage() const;
        void AppendWarning( const gdChar* msg, ... ) const;
        const gdChar* GetWarningMessages() const;
        const gdByte* GetMD5Digest() const;
        hUint32 AddBuildDependency( const hChar* resType, const hChar* depName, const hChar* inputPath );
        gdParameterValue* GetDependencyParameter( hUint32 depID, const hChar* name );
        const gdDependencyArrayType* GetDependencyMap() const;
        void TouchFileIntoBuildCache( const hChar* path );
        virtual void BuildResource() = 0;
        virtual void CleanUpFromBuild() = 0;

    private:
        // Hide stupid ms warning C4251 (XXX has no DLL interface warning)
#pragma warning( push )
#pragma warning( disable: 4251 )

        gdPREVENT_COPY( gdResourceBuilderBase );

        const gdPlugInInformation&      plugInInfo_;
        gdParametersArrayType           parameters_;
        gdUniqueResourceID              uri_;
        gdTimestampInfo                 timestampInfo_;
        gdFileHandle*                   inputFile_;
        gdFileHandle*                   outputFile_;
        Heart::hSerialiser              serialiserObject_;
        gdDependencyArrayType           dependencies_;
        const gdResorucePlugInMap&      plugInInfoMap_;
        mutable gdString                warningMsgs_;
        mutable gdString                errorMsg_;


        //Stop Hiding warning C4251
#pragma warning ( pop )
    };
}

#endif // GDRESOURCEBUILDERBASE_H__