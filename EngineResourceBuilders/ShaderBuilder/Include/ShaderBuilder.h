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
#include "Heart.h"


class ShaderProgramBuilder : public GameData::gdResourceBuilderBase
{
public:
    ShaderProgramBuilder( const GameData::gdResourceBuilderConstructionInfo& resBuilderInfo );
    ~ShaderProgramBuilder();
    void    BuildResource();

    void ParseVertexInputFormat( D3D11_SHADER_DESC &desc, ID3D11ShaderReflection* reflect );

    void    CleanUpFromBuild();

    static const hChar* ParameterName_Entry;
    static const hChar* ParameterName_Profile;
    static const hChar* ParameterName_Type;
    static const hChar* ParameterName_DebugInfo;

private:

    void        MapCgPassStateToRuntimeState( Heart::hMaterialTechniquePass* lhs, CGstateassignment rhs );
    void        MapCgSamplerStateToRuntimeState( Heart::hSamplerParameter* lhs, CGstateassignment rhs );

    hChar*                  cgFXSource;
    Heart::hShaderProgram   outputProgram_;
    ID3DBlob*               result_;


};


#endif // MATERIALBUILDER_H__