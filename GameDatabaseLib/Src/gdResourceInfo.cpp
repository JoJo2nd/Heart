/********************************************************************

	filename: 	gdResourceInfo.cpp	
	
	Copyright (c) 31:10:2011 James Moran
	
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

namespace GameData
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    GameData::gdResourceInfo::gdResourceInfo()
    {

    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    GameData::gdResourceInfo::~gdResourceInfo()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const GameData::gdUniqueResourceID* GameData::gdResourceInfo::GetResourceID() const
    {
        return &resID_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void GameData::gdResourceInfo::SetResourceID( const gdUniqueResourceID& id )
    {
        resID_ = id;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void GameData::gdResourceInfo::SetInputFilePath( const gdChar* inputPath )
    {
        inputPath_ = inputPath;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const gdChar* gdResourceInfo::GetInputFilePath() const
    {
        return inputPath_.c_str();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError GameData::gdResourceInfo::SetResourceType( const gdChar* typeName, gdGameDatabaseObject* database )
    {
        const gdPlugInInformation* info;
        if ( database->GetPlugInInfo( typeName, &info ) == gdERROR_OK )
        {
            parameters_.clear();
            ApplyResourceParametersFromResourceType( *info );
            resourceType_ = typeName;
            return gdERROR_OK;
        }
        return gdERROR_INVALID_PARAM;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const gdChar* GameData::gdResourceInfo::GetResourceTypeName() const
    {
        if ( resourceType_.empty() )
            return NULL;
        else
            return resourceType_.c_str();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdParameterPair* gdResourceInfo::GetParameterPair( const gdChar* name )
    {
        gdUint size = parameters_.size();
        for ( gdUint i = 0; i < size; ++i )
        {
            if ( strcmp( parameters_[i].GetName(), name ) == 0 )
            {
                return &parameters_[i];
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdError gdResourceInfo::ApplyResourceParametersFromResourceType( const gdPlugInInformation& resPlugIn )
    {
        gdParameterDefinitionArray paramDef;
        resPlugIn.GetDefaultParameterArray( paramDef );
        gdUint size = paramDef.size();

        if ( resourceType_ != resPlugIn.GetResourceTypeName() )
        {
            resourceType_ = resPlugIn.GetResourceTypeName();
        }

        for ( gdUint i = 0; i < size; ++i )
        {
            gdParameterPair* param = GetParameterPair( paramDef[i].GetName() );

            if ( !param )
            {
                gdParameterPair pp;
                pp.SetName( paramDef[i].GetName() );
                pp.SetParameterDefinition( paramDef[i] );
                *pp.GetValue() = paramDef[i].GetDefaultValue();
                parameters_.push_back( pp );
            }
        }

        size = parameters_.size();
        for ( gdUint i = 0; i < size; )
        {
            gdBool found = false;
            gdUint defsize = paramDef.size();
            for ( gdUint i2 = 0; i2 < defsize; ++i2 )
            {
                if ( strcmp( parameters_[i].GetName(), paramDef[i2].GetName() ) == 0 )
                {
                    found = true;
                    break;
                }
            }

            if ( !found )
            {
                parameters_[i] = parameters_[--size];
                parameters_.pop_back();
            }
            else
            {
                ++i;
            }
        }


        return gdERROR_OK;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdParametersArrayType* gdResourceInfo::GetResourceParameterArray()
    {
        return &parameters_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const gdParametersArrayType* gdResourceInfo::GetResourceParameterArray() const
    {
        return &parameters_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    gdParameterValue* gdResourceInfo::GetParameter( const gdChar* name )
    {
        gdParameterPair* param = GetParameterPair( name );
        if ( !param )
            return NULL;

        return param->GetValue();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const gdUint32 gdResourceInfo::GetResourceParameterCRC() const
    {
        gdUint32 crc32;
        cyStartCRC32( &crc32, (const gdByte*)resID_.GetResourcePath(), strlen( resID_.GetResourcePath() ) );
        cyContinueCRC32( &crc32, (const gdByte*)resID_.GetResourceName(), strlen( resID_.GetResourceName() ) );
        cyContinueCRC32( &crc32, (const gdByte*)GetInputFilePath(), strlen( GetInputFilePath() ) );
        for ( gdUint32 i = 0; i < parameters_.size(); ++i )
        {
            const gdChar* val = parameters_[i].GetValue()->ConvertToString();
            cyContinueCRC32( &crc32, (const gdByte*)val, strlen( val ) );
        }
        return cyFinaliseCRC32( &crc32 );
    }

}