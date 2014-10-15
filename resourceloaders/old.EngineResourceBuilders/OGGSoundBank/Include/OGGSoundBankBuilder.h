/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef OGGSOUNDBANKBUILDER_H__
#define OGGSOUNDBANKBUILDER_H__

#include <stdio.h>
#include <vector>
#include <string>
#include "GameDatabaseLib.h"
#include "Heart.h"


class OGGSoundBankBuilder : public GameData::gdResourceBuilderBase
{
public:
    OGGSoundBankBuilder( const GameData::gdResourceBuilderConstructionInfo& resBuilderInfo );
    ~OGGSoundBankBuilder();

    void    BuildResource();
    void    CleanUpFromBuild();

private:

    hUint32     xmlsize_;
    hChar*      xmlDocStr_;

};

#endif // OGGSOUNDBANKBUILDER_H__