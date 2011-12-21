/********************************************************************

	filename: 	gdResourceBuilderConstructionInfo.h	
	
	Copyright (c) 26:11:2011 James Moran
	
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

#ifndef GDRESOURCEBUILDERCONSTRUCTIONINFO_H__
#define GDRESOURCEBUILDERCONSTRUCTIONINFO_H__

namespace GameData
{

    struct gdResourceBuilderConstructionInfo
    {
        gdResourceBuilderConstructionInfo( 
            const gdPlugInInformation&      plugInInfo,
            const gdResorucePlugInMap&      plugInMap,
            const gdParametersArrayType&    parameters,
            const gdUniqueResourceID&       uri,
            gdFileHandle* const             inputFile,
            gdFileHandle* const             outputFile,
            gdUint32                        resParamCRC )
            : plugInInfo_( plugInInfo )
            , resourcePlugInMap_( plugInMap )
            , parameters_( parameters )
            , uri_( uri )
            , inputFile_( inputFile )
            , outputFile_( outputFile )
            , resParamCRC_( resParamCRC )
        {

        }

        const gdPlugInInformation&      plugInInfo_;
        const gdResorucePlugInMap&      resourcePlugInMap_;
        const gdParametersArrayType&    parameters_;
        const gdUniqueResourceID&       uri_;
        gdFileHandle* const             inputFile_;
        gdFileHandle* const             outputFile_;
        gdUint32                        resParamCRC_;
    };

}

#endif // GDRESOURCEBUILDERCONSTRUCTIONINFO_H__