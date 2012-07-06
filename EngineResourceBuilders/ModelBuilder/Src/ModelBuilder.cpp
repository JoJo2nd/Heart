/********************************************************************

	filename: 	ModelBuilder.cpp	
	
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


#include "ModelBuilder.h"
#include "Heart.h"
#include "aiScene.h"
#include "aiPostProcess.h"
#include "aiMaterial.h"
#include "aiMatrix4x4.h"

const gdChar* ModelBuilder::ParameterName_GenerateOctTree   = "Build OctTree";
const gdChar* ModelBuilder::ParameterName_PropsXML          = "Property Sheet";
const gdChar* ModelBuilder::ParameterName_ExportLights      = "Export Lights";
const gdChar* ModelBuilder::ParameterName_ExportCameras     = "Export Cameras";
const gdChar* ModelBuilder::ParameterName_SwapYZ            = "Swap YZ Axes";


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ModelBuilder::ModelBuilder( const GameData::gdResourceBuilderConstructionInfo& resBuilderInfo )
    : gdResourceBuilderBase( resBuilderInfo )
    , sceneData_(NULL)
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ModelBuilder::~ModelBuilder()
{
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ModelBuilder::BuildResource()
{
    vertexBufferCount_ = 0;

    LoadPropsFile();

    sceneData_ = aiImportFile( 
        GetInputFile()->GetPath(),
        aiProcess_GenNormals                |
        aiProcess_CalcTangentSpace          |
        aiProcess_Triangulate               |
        //aiProcess_JoinIdenticalVertices     |
        aiProcess_ConvertToLeftHanded       |
        aiProcess_RemoveRedundantMaterials  |
        aiProcess_LimitBoneWeights          |
        aiProcess_FindInstances             |
        //aiProcess_OptimizeGraph             | we'll lose important names with this
        aiProcess_OptimizeMeshes            |
        aiProcess_SortByPType );

    if (!sceneData_->HasMeshes())
        ThrowFatalError("No Meshes in 3D scene");

    meshes_.Reserve(sceneData_->mNumMeshes);
    for (hUint32 i=0; i < sceneData_->mNumMeshes; ++i)
    {
        aiMesh* mesh = sceneData_->mMeshes[i];
        if (mesh->GetNumColorChannels() > 1)
            AppendWarning("Mesh %s has too many colour channels, only the first will be used", mesh->mName);
        if (mesh->GetNumUVChannels() > 8)
            AppendWarning("Mesh %s has too many UV channels, channels 8 to %d will not be used", mesh->mName, mesh->GetNumUVChannels());

        meshes_.PushBack(CreateMeshInfo(mesh));
    }

//     aiString matname;
//     for (hUint32 i = 0; i < sceneData_->mNumMaterials; ++i)
//     {
//         sceneData_->mMaterials[i]->Get(AI_MATKEY_NAME,matname);
//         AppendWarning("Found material by name: %s", matname.data);
//         Heart::hStrWildcardMatch( "mat_GuardHouse-*", matname.data);
//     }

    aiNode* root = sceneData_->mRootNode;
    aiNode* renderRoot = NULL;
    aiNode* physicsRoot = NULL;
    for (hUint32 i = 0; i < root->mNumChildren; ++i)
    {
        if (Heart::hStrICmp(root->mChildren[i]->mName.data, "render") == 0)
        {
            renderRoot = root->mChildren[i];
        }

        if (Heart::hStrICmp(root->mChildren[i]->mName.data, "physics") == 0)
        {
            physicsRoot = root->mChildren[i];
        }
    }

    if (renderRoot)
    {
        WalkRenderSceneGraph(renderRoot, NULL);
    }

    // push the meshes into the scene def
    sceneDef_.indexBufferData_.Resize(indexBuffers_.size());
    for ( hUint32 i = 0, c = indexBuffers_.size(); i < c; ++i)
    {
        sceneDef_.indexBufferData_[i].index_ = new hByte[indexBuffers_[i].sizeBytes_];
        Heart::hMemCpy(sceneDef_.indexBufferData_[i].index_, indexBuffers_[i].data_, indexBuffers_[i].sizeBytes_ );
    }

    sceneDef_.vertexBufferData_.Resize(vertexBufferCount_);
    for ( VertexBufferMap::iterator i = vertexBufferMap_.begin(), c = vertexBufferMap_.end(); i != c; ++i)
    {
        hUint32 format = i->first;
        for ( VertexBufferList::iterator vb = i->second.begin(), vbe = i->second.end(); vb != vbe; ++vb )
        {
            ResourceAssert(format == vb->vtxFmt_);
            sceneDef_.vertexBufferData_[i].vertexFormat_ = vb->vtxFmt_;
            sceneDef_.vertexBufferData_[i].sizeBytes_ = vb->sizeBytes_;
            sceneDef_.vertexBufferData_[i].vertex_ = new hByte[vb->sizeBytes_];
            Heart::hMemCpy(sceneDef_.vertexBufferData_[i].vertex_, vb->data_, vb->sizeBytes_);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ModelBuilder::CleanUpFromBuild()
{
    if (sceneData_)
    {
        //aiReleaseImport(sceneData_);
        sceneData_ = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ModelBuilder::LoadPropsFile()
{
    Heart::hXMLDocument propsxml;
    hUint32 xmlsize;
    hChar* xmlDocStr;

    propsFile_ = OpenFile(GetParameter(ParameterName_PropsXML).GetAsFilepath());
    if (propsFile_->IsValid())
    {
        xmlsize = (hUint32)propsFile_->GetFileSize();
        xmlDocStr = (hChar*)hMalloc(xmlsize+1);
        propsFile_->Read(xmlDocStr, xmlsize);
        xmlDocStr[xmlsize] = 0;
        propsxml.parse<0>(xmlDocStr);

        Heart::hXMLGetter materials = Heart::hXMLGetter(&propsxml).FirstChild("scene").FirstChild("materials");

        for ( Heart::hXMLGetter link = materials.FirstChild("link"); link.ToNode(); link = link.NextSibling() )
        {
            ResourceAssert(link.GetAttribute("source"), "Couldn't find attribute \"source\" in element %s", link.ToNode()->name());
            ResourceAssert(link.GetAttribute("dest"), "Couldn't find attribute \"dest\" in element %s", link.ToNode()->name());

            MaterialLink matlink;
            matlink.wildcardMatch_ = link.GetAttributeString("source");
            matlink.cgfxPath_      = link.GetAttributeString("dest");

            materialLinks_.push_back(matlink);
        }
    }
    else
    {
        CloseFile(propsFile_);
        ThrowFatalError("Failed to open props file.");
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ModelBuilder::BuildVertexFormatFromMesh( const aiMesh* mesh ) const
{
    hUint32 fmt = 0;
    hUint32 uvs = hMin(mesh->GetNumUVChannels(),8);

    fmt |= mesh->HasPositions()             ? Heart::hrVF_XYZ : 0;
    fmt |= mesh->HasNormals()               ? Heart::hrVF_NORMAL : 0;
    fmt |= mesh->HasVertexColors(0)         ? Heart::hrVF_COLOR : 0;
    fmt |= mesh->HasTangentsAndBitangents() ? Heart::hrVF_BINORMAL|Heart::hrVF_TANGENT : 0;
    fmt |= uvs > 0                          ? Heart::hrVE_1UV << (uvs-1) : 0;

    return fmt;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ModelBuilder::BuildVertexStrideFromMesh( const aiMesh* mesh ) const
{
    hUint32 stride = 0;

    stride += mesh->HasPositions()             ? sizeof(hFloat)*3 : 0;
    stride += mesh->HasNormals()               ? sizeof(hFloat)*3 : 0;
    stride += mesh->HasVertexColors(0)         ? sizeof(hFloat)*4 : 0;
    stride += mesh->HasTangentsAndBitangents() ? sizeof(hFloat)*6 : 0;
    stride += hMin(mesh->GetNumUVChannels(),8)*sizeof(hFloat)*2;

    return stride;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ModelBuilder::MeshInfo ModelBuilder::CreateMeshInfo( const aiMesh* mesh )
{
    MeshInfo minfo;

    AppendVertexBuffer(mesh, &minfo.vertexBufferID_, &minfo.vtxFormat_, &minfo.startIndex_, &minfo.aabb_);
    AppendIndexBuffer(mesh, minfo.startIndex_, &minfo.indexBufferID_, &minfo.nPrims_, &minfo.type_);

    minfo.materialIndex_ = GetValidMaterial(mesh->mMaterialIndex);

    return minfo;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ModelBuilder::AppendIndexBuffer( const aiMesh* mesh, hUint32 startIdx, hUint32* outIdxBufID, hUint32* nPrims, hUint32* type )
{
    IndexBufferData newbuf;
    newbuf.sizeBytes_ = mesh->mNumFaces*3*sizeof(hUint16);
    newbuf.data_      = (hUint16*)hMalloc(newbuf.sizeBytes_);
    hUint16* ptr = newbuf.data_;

    for (hUint32 i = 0; i < mesh->mNumFaces; ++i)
    {
        ResourceAssert( mesh->mFaces[i].mNumIndices == 3, "Non-triangle mesh...");
        ptr[0] = startIdx+mesh->mFaces[i].mIndices[0];
        ptr[1] = startIdx+mesh->mFaces[i].mIndices[1];
        ptr[2] = startIdx+mesh->mFaces[i].mIndices[2];

        ptr += 3;
    }

    hcAssert((hByte*)ptr <= ((hByte*)newbuf.data_)+newbuf.sizeBytes_);

    *outIdxBufID = indexBuffers_.size();
    indexBuffers_.push_back(newbuf);
    *nPrims = mesh->mNumFaces;
    switch(mesh->mPrimitiveTypes)
    {
    case aiPrimitiveType_TRIANGLE:
        *type = Heart::PRIMITIVETYPE_TRILIST;
        break;
    case aiPrimitiveType_LINE:
    case aiPrimitiveType_POINT:
    case aiPrimitiveType_POLYGON:
    default:
        ThrowFatalError("Primative type %d not supported", mesh->mPrimitiveTypes);
        break;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ModelBuilder::AppendVertexBuffer(const aiMesh* mesh, hUint32* vertexBufferID_, hUint32* vtxFormat, hUint32* outStartIdx, Heart::hAABB* outaabb)
{
    VertexBufferData vtxBuf;
    hBool swapYZ = GetParameter(ParameterName_ExportCameras).GetAsBool();
    hUint32 stride = BuildVertexStrideFromMesh(mesh);
    vtxBuf.vtxFmt_ = BuildVertexFormatFromMesh(mesh);
    vtxBuf.sizeBytes_ = stride*mesh->mNumVertices;
    vtxBuf.data_ = hMalloc(vtxBuf.sizeBytes_);

    if (vertexBufferMap_.find(vtxBuf.vtxFmt_) == vertexBufferMap_.end())
    {
        vertexBufferMap_[vtxBuf.vtxFmt_] = VertexBufferList();
    }

    /*
         Vertex Buffer layout: MUST match layout in engine!!!
            Position,
            Normal,
            Tangent,
            Binormal,
            Colour,
            UV(1-8)
    */
    hFloat* ptr = (hFloat*)vtxBuf.data_;

    outaabb->c_ = Heart::hVec3(0,0,0);
    outaabb->r_ = Heart::hVec3(0,0,0);

    for (hUint32 i = 0; i < mesh->mNumVertices; ++i)
    {
        if (mesh->HasPositions())
        {
            if (swapYZ)
            {
                ptr[0] = mesh->mVertices[i].x;
                ptr[1] = mesh->mVertices[i].z;
                ptr[2] = mesh->mVertices[i].y;
            }
            else
            {
                ptr[0] = mesh->mVertices[i].x;
                ptr[1] = mesh->mVertices[i].y;
                ptr[2] = mesh->mVertices[i].z;
            }

            Heart::hAABB::expandBy(*outaabb, Heart::hVec3(ptr[0],ptr[1],ptr[3]));
            ptr += 3;
        }
        if (mesh->HasNormals())
        {
            if (swapYZ)
            {
                ptr[0] = mesh->mNormals[i].x;
                ptr[1] = mesh->mNormals[i].z;
                ptr[2] = mesh->mNormals[i].y;
            }
            else
            {
                ptr[0] = mesh->mNormals[i].x;
                ptr[1] = mesh->mNormals[i].y;
                ptr[2] = mesh->mNormals[i].z;
            }
            ptr += 3;
        }
        if (mesh->HasTangentsAndBitangents())
        {
            if (swapYZ)
            {
                ptr[0] = mesh->mTangents[i].x;
                ptr[1] = mesh->mTangents[i].z;
                ptr[2] = mesh->mTangents[i].y;
                ptr[3] = mesh->mBitangents[i].x;
                ptr[4] = mesh->mBitangents[i].z;
                ptr[5] = mesh->mBitangents[i].y;
            }
            else
            {
                ptr[0] = mesh->mTangents[i].x;
                ptr[1] = mesh->mTangents[i].y;
                ptr[2] = mesh->mTangents[i].z;
                ptr[3] = mesh->mBitangents[i].x;
                ptr[4] = mesh->mBitangents[i].y;
                ptr[5] = mesh->mBitangents[i].z;
            }
            ptr += 6;
        }
        if (mesh->HasVertexColors(0))
        {
            ptr[0] = mesh->mColors[0][i].r;
            ptr[1] = mesh->mColors[0][i].g;
            ptr[2] = mesh->mColors[0][i].b;
            ptr[4] = mesh->mColors[0][i].a;

            ptr += 4;
        }

        hUint32 maxuvs = hMin(mesh->GetNumUVChannels(),8);
        for (hUint32 uvs=0; uvs < maxuvs; ++uvs)
        {
            ptr[0] = mesh->mTextureCoords[uvs][i].x;
            ptr[1] = mesh->mTextureCoords[uvs][i].y;

            ptr += 2;
        }

        hcAssert((hByte*)ptr <= ((hByte*)vtxBuf.data_)+vtxBuf.sizeBytes_);
    }

    hcAssert((hByte*)ptr <= ((hByte*)vtxBuf.data_)+vtxBuf.sizeBytes_);

    *vtxFormat = vtxBuf.vtxFmt_;
    *vertexBufferID_ = vertexBufferMap_[vtxBuf.vtxFmt_].size();
    vertexBufferMap_[vtxBuf.vtxFmt_].push_back(vtxBuf);
    *outStartIdx = 0;

    ++vertexBufferCount_;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ModelBuilder::GetValidMaterial( hUint32 materialIndex ) const
{
    ResourceAssert(materialIndex < sceneData_->mNumMaterials, "Material Index too large");
    aiString name;
    
    sceneData_->mMaterials[materialIndex]->Get(AI_MATKEY_NAME, name);
    
    for (hUint32 i = 0, c = materialLinks_.size(); i < c; ++i)
    {   
        aiString matname;
        if (Heart::hStrWildcardMatch(materialLinks_[i].wildcardMatch_.c_str(),name.data))    
        {
            return i;
        }
    }

    ThrowFatalError("Couldn't find material to match against name %s in props file", name.data);
    return ~0U;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ModelBuilder::WalkRenderSceneGraph(const aiNode* node, Heart::hLODGroup* lodGroup)
{
    Heart::hLODGroup newlodGroup;
    hBool useLodGroup = hFalse;

    if (node->mNumMeshes > 0)
    {
        aiMatrix4x4 xform = GetWorldTransform(node);
        //do transpose here?
        Heart::hNodeLink nodelink;
        nodelink.sceneNodeID    = sceneDef_.sceneNodeComponents_.GetSize();

        sceneDef_.sceneNodeComponents_.Resize(sceneDef_.sceneNodeComponents_.GetSize()+1);
        Heart::hSceneNodeComponent& sceneComp = sceneDef_.sceneNodeComponents_[sceneDef_.sceneNodeComponents_.GetSize()-1];
        sceneComp.localMatrix_._11 = xform.a1;
        sceneComp.localMatrix_._12 = xform.a2;
        sceneComp.localMatrix_._13 = xform.a3;
        sceneComp.localMatrix_._14 = xform.a4;

        sceneComp.localMatrix_._21 = xform.b1;
        sceneComp.localMatrix_._22 = xform.b2;
        sceneComp.localMatrix_._23 = xform.b3;
        sceneComp.localMatrix_._24 = xform.b4;

        sceneComp.localMatrix_._31 = xform.c1;
        sceneComp.localMatrix_._32 = xform.c2;
        sceneComp.localMatrix_._33 = xform.c3;
        sceneComp.localMatrix_._34 = xform.c4;

        sceneComp.localMatrix_._41 = xform.d1;
        sceneComp.localMatrix_._42 = xform.d2;
        sceneComp.localMatrix_._43 = xform.d3;
        sceneComp.localMatrix_._44 = xform.d4;
        Heart::hStrCopy(sceneComp.name_.GetBuffer(), sceneComp.name_.GetMaxSize(), node->mName.data);

        if (!useLodGroup)
        {
            nodelink.renderNodeID   = sceneDef_.renderModels_.GetSize();
            sceneDef_.renderModels_.Resize(sceneDef_.renderModels_.GetSize()+1);
            Heart::hRenderModelComponent& renderComp = sceneDef_.renderModels_[sceneDef_.renderModels_.GetSize()-1];

            for (hUint32 i = 0; i < node->mNumMeshes; ++i)
            {
                //Add mesh
                MeshInfo& minfo = meshes_[node->mMeshes[i]];
                Heart::hRenderable r;
                
                r.SetPrimativeType((Heart::PrimitiveType)minfo.type_);
                r.SetStartIndex(minfo.startIndex_);
                r.SetPrimativeCount(minfo.nPrims_);
                r.SetIndexBuffer((Heart::hIndexBuffer*)minfo.indexBufferID_);
                r.SetVertexBuffer((Heart::hVertexBuffer*)minfo.vertexBufferID_);
                r.SetMaterial((Heart::hMaterial*)minfo.materialIndex_);
                r.SetAABB(minfo.aabb_);

                renderComp.lodGroup_.lodLevels_.At(0).renderObjects_.PushBack(r);
            }

            renderComp.lodGroup_.lodLevels_.At(0).minRange_ = Heart::Limits::FLOAT_MAX;
            renderComp.transform_ = sceneComp.localMatrix_;
        }
        else
        {
            //add to lod group
            //get level from name
            ThrowFatalError("LODS not supported yet");
        }

        sceneDef_.nodeLinks_.PushBack(nodelink);
    }

    //TODO: lod groups
    //useLodGroup = Heart::hStrWildcardMatch("LOD_*", node->mName.data) ? hTrue : hFalse;
    ResourceAssert((useLodGroup && !lodGroup)|| !useLodGroup, "LOD group found within another LOD group");

    for (hUint32 i = 0; i < node->mNumChildren; ++i)
    {
        WalkRenderSceneGraph(node->mChildren[i], useLodGroup ? &newlodGroup : NULL);
    }

    if (useLodGroup)
    {
        //
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

aiMatrix4x4 ModelBuilder::GetWorldTransform( const aiNode* node ) const
{
    aiMatrix4x4 xform;//<-- constructs to identity
    for (const aiNode* parent = node; parent; parent = parent->mParent)
    {
        xform = parent->mTransformation * xform;
    }

    return xform;
}

