/********************************************************************

	filename: 	WorldObjectBuilder.h	
	
	Copyright (c) 28:3:2012 James Moran
	
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
#ifndef WORLDOBJECTBUILDER_H__
#define WORLDOBJECTBUILDER_H__

#include <stdio.h>
#include <vector>
#include <string>
#include "GameDatabaseLib.h"
#include "Heart.h"
#include "hRapidXML.h"


class WorldObjectBuilder : public GameData::gdResourceBuilderBase
{
public:
    WorldObjectBuilder( const GameData::gdResourceBuilderConstructionInfo& resBuilderInfo );
    ~WorldObjectBuilder();
    void    BuildResource();
    void    CleanUpFromBuild();

private:

    struct NodeLinks
    {
        rapidxml::xml_node<>*   l1_;
        rapidxml::xml_node<>*   l2_;
    };

    struct XMLDoc
    {
        Heart::hXMLDocument*   xml_;
        hByte*           data_;
    };

    typedef std::vector< XMLDoc >  XMLDocArray;

    void ParseFile( XMLDoc& doc, GameData::gdFileHandle* file );

    XMLDoc          root_;
    XMLDocArray     docs_;
    hUint32         totalsize_;
    void*           data_;
};

#endif // WORLDOBJECTBUILDER_H__