/********************************************************************

	filename: 	ModelBuilder.h	
	
	Copyright (c) 19:5:2012 James Moran
	
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


#ifndef TextureBuilder_h__
#define TextureBuilder_h__

#include <stdio.h>
#include <vector>
#include <string>
#include "assimp.h"
#include "GameDatabaseLib.h"

struct aiScene;
struct aiMesh;
struct aiNode;

namespace Heart
{
    class hLODGroup;
}

#define ResourceAssert(cond, msg, ...) if (!(cond)) ThrowFatalError( #cond " - " msg, __VA_ARGS__ )

class ModelBuilder : public GameData::gdResourceBuilderBase
{
public:
    ModelBuilder( const GameData::gdResourceBuilderConstructionInfo& resBuilderInfo );
    ~ModelBuilder();
    void    BuildResource();
    void    CleanUpFromBuild();

    static const gdChar*                ParameterName_GenerateOctTree;
    static const gdChar*                ParameterName_PropsXML;
    static const gdChar*                ParameterName_ExportLights;
    static const gdChar*                ParameterName_ExportCameras;
    static const gdChar*                ParameterName_SwapYZ;

private:
    
    typedef std::string String;

    struct MaterialLink
    {
        String      wildcardMatch_;
        String      cgfxPath_;
    };

    struct IndexBufferData
    {
        hUint32  sizeBytes_;
        hUint16* data_;
    };

    struct VertexBufferData
    {
        hUint32 vtxFmt_;
        hUint32 sizeBytes_;
        void*   data_;
    };

    struct MeshInfo
    {
        hUint32 startIndex_;
        hUint32 indexBufferID_;
        hUint32 vtxFormat_;
        hUint32 vertexBufferID_;
        hUint32 nPrims_;
        hUint32 type_;
        hUint32 materialIndex_;
        Heart::hAABB aabb_;
    };

    typedef std::vector< VertexBufferData >         VertexBufferList;
    typedef std::vector< MaterialLink >             MaterialLinkArray;
    typedef Heart::hVector< MeshInfo >              MeshInfoArray;
    typedef std::map< hUint32, VertexBufferList >   VertexBufferMap;
    typedef std::vector< IndexBufferData >          IndexBufferArray;

    void            LoadPropsFile();
    hUint32         BuildVertexFormatFromMesh(const aiMesh* mesh) const;
    hUint32         BuildVertexStrideFromMesh(const aiMesh* mesh) const;
    MeshInfo        CreateMeshInfo(const aiMesh* mesh);
    void            AppendIndexBuffer(const aiMesh* mesh, hUint32 startIdx, hUint32* outIdxBufID, hUint32* nPrims, hUint32* type);
    void            AppendVertexBuffer(const aiMesh* mesh, hUint32* vertexBufferID_, hUint32* vtxFormat, hUint32* outStartIdx, Heart::hAABB* outaabb);
    hUint32         GetValidMaterial(hUint32 materialIndex) const;
    void            WalkRenderSceneGraph(const aiNode* node, Heart::hLODGroup* lodGroup);
    aiMatrix4x4     GetWorldTransform(const aiNode* node) const;

    const aiScene*          sceneData_;
    GameData::gdFileHandle* propsFile_;
    MaterialLinkArray       materialLinks_;
    IndexBufferArray        indexBuffers_;
    hUint32                 vertexBufferCount_;
    VertexBufferMap         vertexBufferMap_;
    MeshInfoArray           meshes_;
    Heart::hSceneDefinition sceneDef_;
};

#endif // TextureBuilder_h__