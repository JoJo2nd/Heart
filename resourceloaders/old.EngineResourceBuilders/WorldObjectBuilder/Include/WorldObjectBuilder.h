/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef WORLDOBJECTBUILDER_H__
#define WORLDOBJECTBUILDER_H__

#include <stdio.h>
#include <vector>
#include <string>
#include "GameDatabaseLib.h"
#include "Heart.h"


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