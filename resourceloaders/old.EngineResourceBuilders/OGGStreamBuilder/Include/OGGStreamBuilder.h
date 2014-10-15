/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef OGGSTREAMBUILDER_H__
#define OGGSTREAMBUILDER_H__

#include <stdio.h>
#include <vector>
#include <string>
#include "GameDatabaseLib.h"
#include "Heart.h"


class OGGStreamBuilder : public GameData::gdResourceBuilderBase
{
public:
    OGGStreamBuilder( const GameData::gdResourceBuilderConstructionInfo& resBuilderInfo );
    ~OGGStreamBuilder();
    void    BuildResource();
    void    CleanUpFromBuild();

private:

};

#endif // OGGSTREAMBUILDER_H__

