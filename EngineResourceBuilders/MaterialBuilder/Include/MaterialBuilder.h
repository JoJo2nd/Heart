/********************************************************************
	created:	2010/10/17
	created:	17:10:2010   11:37
	filename: 	MaterialBuilder.h	
	author:		James
	
	purpose:	
*********************************************************************/

#ifndef MATERIALBUILDER_H__
#define MATERIALBUILDER_H__

#include <stdio.h>
#include <vector>
#include <string>
#include "GameDatabaseLib.h"
#include <cg/cg.h>
#include <cg/cgGL.h>
#include <Cg/cgD3D9.h>
#include <Cg/cgD3D10.h>
#include <Cg/cgD3D11.h>


namespace Heart
{
    class hMaterialTechniquePass;
    struct hSamplerParameter;
}

class MaterialEffectBuilder : public GameData::gdResourceBuilderBase
{
public:
    MaterialEffectBuilder( const GameData::gdResourceBuilderConstructionInfo& resBuilderInfo );
    ~MaterialEffectBuilder();
    void    BuildResource();
    void    CleanUpFromBuild();
    hBool   GetCgIncludeSource(const boost::filesystem::path& filename, gdString& outSource, boost::filesystem::path& fileid);

    static const hChar* ParameterName_DebugInfo;

private:

    void        MapCgPassStateToRuntimeState( Heart::hMaterialTechniquePass* lhs, CGstateassignment rhs );
    void        MapCgSamplerStateToRuntimeState( Heart::hSamplerParameter* lhs, CGstateassignment rhs );

    typedef std::list< boost::filesystem::path > IncSearchPathList;

    hChar*              cgFXSource_;
    IncSearchPathList   incPaths_;

};


#endif // MATERIALBUILDER_H__