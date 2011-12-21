/********************************************************************

	filename: 	ActionExportScene.cpp	
	
	Copyright (c) 6:3:2011 James Moran
	
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

#include "stdafx.h"
#include "ActionExportScene.h"
#include "tinyxml.h"
#include "SceneNodeMeshCollection.h"
#include "SceneNodeLight.h"
#include "SceneNodeCamera.h"
#include "DataMesh.h"
#include "DataLight.h"
#include "DataCamera.h"
#include "DataMaterial.h"
#include "DataTexture.h"
#include <stack>

namespace Action
{
namespace 
{
	// Tables to match enum values
	const char* BlendFunctionStrings[] =
	{
		"none",
		"add",
		"sub",
		"min",
		"max"
	};
	const char* BlendOpString[] = 
	{
		"zero",				//MAT_BLEND_OP_ZERO,
		"one",				//MAT_BLEND_OP_ONE,
		"srccolour",		//MAT_BLEND_OP_SRC_COLOUR,
		"inversesrccolour",	//MAT_BLEND_OP_INVSRC_COLOUR,
		"destcolour",		//MAT_BLEND_OP_DEST_COLOUR,
		"inversedestcolour",//MAT_BLEND_OP_INVDEST_COLOUR,
		"srcalpha",			//MAT_BLEND_OP_SRC_ALPHA,
		"inversesrcalpha",	//MAT_BLEND_OP_INVSRC_ALPHA,
		"destalpha",		//MAT_BLEND_OP_DEST_ALPHA,
		"inversedestalpha",	//MAT_BLEND_OP_INVDEST_ALPHA,
	};
	const char* ZCompareStrings[] = 
	{
		"never",		//MAT_Z_CMP_NEVER,
		"less",			//MAT_Z_CMP_LESS,
		"equal",		//MAT_Z_CMP_EQUAL,
		"lessequal",	//MAT_Z_CMP_LESSEQUAL,
		"greater",		//MAT_Z_CMP_GREATER,
		"notequal",		//MAT_Z_CMP_NOT_EQUAL,
		"greaterequal",	//MAT_Z_CMP_GREATER_EQUAL,
		"always",		//MAT_Z_CMP_ALWAYS,
	};
}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	ExportScene::ExportScene( HScene::SceneGraph* pSceneToExport, wxString* destPath, wxString* name ) :
		pScene_( pSceneToExport )
		,destPath_( *destPath )
		,destName_( *name )
	{
		destPath_.Replace( "\\", "/", true );
		if ( !destPath_.EndsWith( "/" ) )
		{
			destPath_ += "/";
		}

		destPath_ += destName_;

		if ( !destPath_.EndsWith( "/" ) )
		{
			destPath_ += "/";
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	ExportScene::~ExportScene()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ExportScene::Execute()
	{
		std::list< Data::Mesh* > usedMeshes;
		std::list< Data::Material* > usedMaterials;
		std::list< Data::TextureInstance* > usedTextures;

		wxString sceneDir = destPath_;
		sceneDir += _T("scene/");
		wxString meshDir = destPath_;
		meshDir += _T("meshes/");
		wxString materialDir = destPath_;
		materialDir += _T("materials/");
		wxString textureDir = destPath_;
		textureDir += _T("textures/");

		//Export the scene layout to the scene directory
		if ( CreateDirectory( destPath_.wc_str(), NULL ) == FALSE && GetLastError() != ERROR_ALREADY_EXISTS )
		{
			wxMessageDialog msg( NULL, _T( "Error creating export root directory" ) );
			msg.ShowModal();
			return;
		}
		
		//Export the scene layout to the scene directory
		if ( CreateDirectory( sceneDir.wc_str(), NULL ) == FALSE && GetLastError() != ERROR_ALREADY_EXISTS )
		{
			wxMessageDialog msg( NULL, _T( "Error creating scene directory" ) );
			msg.ShowModal();
			return;
		}

		{
			wxString scenefile = sceneDir;
			scenefile += "scene.xml";

			TiXmlDocument doc; 
			doc.SetCondenseWhiteSpace( false );
			TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "yes" ); 
			doc.LinkEndChild( decl );
			
			TiXmlElement* pData = new TiXmlElement( "meshinstances" );
			TiXmlElement* pRoot = new TiXmlElement( "scenenodes" );

			doc.LinkEndChild( pData );
			doc.LinkEndChild( pRoot );

			//VisitScene & export node data
			VisitScene( pScene_, pRoot, &usedMeshes );

			//Export mesh names
			for ( std::list< Data::Mesh* >::iterator i = usedMeshes.begin(); i != usedMeshes.end(); ++i )
			{
				wxString meshFilename;
				meshFilename.Printf( "../meshes/mesh_%03d.xml", (*i)->GetMeshSceneIndex() );
				TiXmlElement* pMeshEle = new TiXmlElement( "meshinstance" );
				TiXmlText* pMeshName = new TiXmlText( meshFilename.c_str() );

				pMeshEle->LinkEndChild( pMeshName );
				pData->LinkEndChild( pMeshEle );
			}

			doc.SaveFile( scenefile.c_str() );
		}

		//Export the mesh instnaces to the meshes directory
		if ( CreateDirectory( meshDir.wc_str(), NULL ) == FALSE && GetLastError() != ERROR_ALREADY_EXISTS )
		{
			wxMessageDialog msg( NULL, _T( "Error creating meshes directory" ) );
			msg.ShowModal();
			return;
		}

		//Export all the meshes we found before when paring the scene
		ExportMeshList(usedMeshes, meshDir, materialDir, usedMaterials);

		//Create the Export Directory for the materials
		if ( CreateDirectory( materialDir.wc_str(), NULL ) == FALSE && GetLastError() != ERROR_ALREADY_EXISTS )
		{
			wxMessageDialog msg( NULL, _T( "Error creating material export directory" ) );
			msg.ShowModal();
			return;
		}

		//Export all materials found during the last export of meshes
		for ( std::list< Data::Material* >::iterator i = usedMaterials.begin(); i != usedMaterials.end(); ++i )
		{
			//Cache the material
			Data::Material* pMat = *i;
			//build the full path name
			wxString materialFullPath;
			materialFullPath.Printf( "%smaterial_%03d.xml", materialDir, pMat->GetMaterialSceneIndex() );

			TiXmlDocument doc; 
			doc.SetCondenseWhiteSpace( false );
			TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "yes" ); 
			TiXmlElement* pMatRootEle = new TiXmlElement( "material" );
			doc.LinkEndChild( decl );
			doc.LinkEndChild( pMatRootEle );

			TiXmlElement* pNameEle = new TiXmlElement( "name" );
			TiXmlText* pName = new TiXmlText( pMat->GetName() );
			pNameEle->LinkEndChild( pName );
			pMatRootEle->LinkEndChild( pNameEle );

			// Add Each Colour element
			TiXmlElement* diffuseEle = new TiXmlElement( "diffuse" );
			diffuseEle->SetDoubleAttribute( "red",		pMat->GetDiffuse().r );
			diffuseEle->SetDoubleAttribute( "green",	pMat->GetDiffuse().g );
			diffuseEle->SetDoubleAttribute( "blue",		pMat->GetDiffuse().b );
			diffuseEle->SetDoubleAttribute( "alpha",	pMat->GetDiffuse().a );
			pMatRootEle->LinkEndChild( diffuseEle );

			TiXmlElement* ambientEle = new TiXmlElement( "ambient" );
			ambientEle->SetDoubleAttribute( "red",		pMat->GetAmbient().r );
			ambientEle->SetDoubleAttribute( "green",	pMat->GetAmbient().g );
			ambientEle->SetDoubleAttribute( "blue",		pMat->GetAmbient().b );
			ambientEle->SetDoubleAttribute( "alpha",	pMat->GetAmbient().a );
			pMatRootEle->LinkEndChild( ambientEle );

			TiXmlElement* specEle = new TiXmlElement( "specular" );
			specEle->SetDoubleAttribute( "red",		pMat->GetSpecular().r );
			specEle->SetDoubleAttribute( "green",	pMat->GetSpecular().g );
			specEle->SetDoubleAttribute( "blue",	pMat->GetSpecular().b );
			specEle->SetDoubleAttribute( "alpha",	pMat->GetSpecular().a );
			pMatRootEle->LinkEndChild( specEle );

			TiXmlElement* emissiveEle = new TiXmlElement( "emissive" );
			emissiveEle->SetDoubleAttribute( "red",		pMat->GetEmissive().r );
			emissiveEle->SetDoubleAttribute( "green",	pMat->GetEmissive().g );
			emissiveEle->SetDoubleAttribute( "blue",	pMat->GetEmissive().b );
			emissiveEle->SetDoubleAttribute( "alpha",	pMat->GetEmissive().a );
			pMatRootEle->LinkEndChild( emissiveEle );

			// Add each render state

			//Add Blend State
			TiXmlElement* pBlendStateEle = new TiXmlElement( "blendstate" );
			pBlendStateEle->SetAttribute( "enable",		pMat->GetBlendFunction() == Data::MAT_BLEND_FUNC_NONE ? "false" : "true" );
			pBlendStateEle->SetAttribute( "srcblend",	BlendOpString[pMat->GetBlendOPSrc()] );
			pBlendStateEle->SetAttribute( "dstblend",	BlendOpString[pMat->GetBlendOPDst()] );
			pBlendStateEle->SetAttribute( "function",	BlendFunctionStrings[pMat->GetBlendFunction()] );
			pMatRootEle->LinkEndChild( pBlendStateEle );

			//Add Z Test State
			TiXmlElement* pZStateEle = new TiXmlElement( "zstate" );
			pZStateEle->SetAttribute( "ztest",		pMat->GetZDepthTest() ? "true" : "false" );
			pZStateEle->SetAttribute( "zwrite",		pMat->GetZWrite() ? "true" : "false" );
			pZStateEle->SetAttribute( "zcompare",	ZCompareStrings[pMat->GetZTestState()] );
			pMatRootEle->LinkEndChild( pZStateEle );

			TiXmlElement* pTwoSidedEle = new TiXmlElement( "twosided" );
			pTwoSidedEle->SetAttribute( "enable", pMat->GetTwoSided() ? "true" : "false" );
			pMatRootEle->LinkEndChild( pTwoSidedEle );

			TiXmlElement* pWireframe = new TiXmlElement( "wireframe" );
			pWireframe->SetAttribute( "enable", pMat->GetWireframe() ? "true" : "false" );
			pMatRootEle->LinkEndChild( pWireframe );

			//Export all texture handles as file names
			for ( u32 i2 = 0; i2 < Data::Material::MAX_DIFFUSE_TEXTURES; ++i2 )
			{
				if ( pMat->GetDiffuseTextureHandle( i2 ) != Data::INVALIDTEXTUREHANDLE )
				{
					Data::TextureInstance* hTex = pMat->GetDiffuseTexture( i2 );
					TiXmlElement* pDiffuseTexEle = new TiXmlElement( "diffusesampler" );
					pDiffuseTexEle->SetAttribute( "index", i2 );
					wxString texPath = "../textures/";//textureDir;
					wxString texBase = hTex->filename_.c_str();
					texBase.Replace( "\\", "/", true );
					texPath += texBase.AfterLast( '/' );
					TiXmlText* pTexPath = new TiXmlText( texPath );
					pDiffuseTexEle->LinkEndChild( pTexPath );
					pMatRootEle->LinkEndChild( pDiffuseTexEle );

					// add texture to list to copy across
					if ( std::find( usedTextures.begin(), usedTextures.end(), hTex ) == usedTextures.end() )
					{
						usedTextures.push_back( hTex );
					}
				}
			}

			if ( pMat->GetNormalTextureHandle() != Data::INVALIDTEXTUREHANDLE )
			{
				Data::TextureInstance* hTex = pScene_->GetTexture( pMat->GetNormalTextureHandle() );
				TiXmlElement* pSamplerEle = new TiXmlElement( "normalsampler" );
				wxString texPath = "../textures/";//textureDir;
				wxString texBase = hTex->filename_.c_str();
				texBase.Replace( "\\", "/", true );
				texPath += texBase.AfterLast( '/' );
				TiXmlText* pTexPath = new TiXmlText( texPath );
				pSamplerEle->LinkEndChild( pTexPath );
				pMatRootEle->LinkEndChild( pSamplerEle );

				// add texture to list to copy across
				if ( std::find( usedTextures.begin(), usedTextures.end(), hTex ) == usedTextures.end() )
				{
					usedTextures.push_back( hTex );
				}
			}

			if ( pMat->GetSpecularTextureHandle() != Data::INVALIDTEXTUREHANDLE )
			{
				Data::TextureInstance* hTex = pScene_->GetTexture( pMat->GetSpecularTextureHandle() );
				TiXmlElement* pSamplerEle = new TiXmlElement( "specularsampler" );
				wxString texPath = "../textures/";//textureDir;
				wxString texBase = hTex->filename_.c_str();
				texBase.Replace( "\\", "/", true );
				texPath += texBase.AfterLast( '/' );
				TiXmlText* pTexPath = new TiXmlText( texPath );
				pSamplerEle->LinkEndChild( pTexPath );
				pMatRootEle->LinkEndChild( pSamplerEle );

				// add texture to list to copy across
				if ( std::find( usedTextures.begin(), usedTextures.end(), hTex ) == usedTextures.end() )
				{
					usedTextures.push_back( hTex );
				}
			}

			if ( pMat->GetLightMapTextureHandle() != Data::INVALIDTEXTUREHANDLE )
			{
				Data::TextureInstance* hTex = pScene_->GetTexture( pMat->GetLightMapTextureHandle() );
				TiXmlElement* pSamplerEle = new TiXmlElement( "lightmapsampler" );
				wxString texPath = "../textures/";//textureDir;
				wxString texBase = hTex->filename_.c_str();
				texBase.Replace( "\\", "/", true );
				texPath += texBase.AfterLast( '/' );
				TiXmlText* pTexPath = new TiXmlText( texPath );
				pSamplerEle->LinkEndChild( pTexPath );
				pMatRootEle->LinkEndChild( pSamplerEle );

				// add texture to list to copy across
				if ( std::find( usedTextures.begin(), usedTextures.end(), hTex ) == usedTextures.end() )
				{
					usedTextures.push_back( hTex );
				}
			}

			//Export the Shader code
			TiXmlElement* pShaderEle = new TiXmlElement( "shader" );
			wxString finalShaderCode = pMat->GetShaderCode();
			finalShaderCode.Replace( "\\n", "\n" );
			TiXmlText* pShader = new TiXmlText( finalShaderCode.c_str() );
			pShaderEle->LinkEndChild( pShader );
			pMatRootEle->LinkEndChild( pShaderEle );

			// finally, save the xml out
			doc.SaveFile( materialFullPath.c_str() );
		}

		{

			//Create the Export Directory for the textures
			if ( CreateDirectory( textureDir.wc_str(), NULL ) == FALSE && GetLastError() != ERROR_ALREADY_EXISTS )
			{
				wxMessageDialog msg( NULL, _T( "Error creating texture export directory" ) );
				msg.ShowModal();
				return;
			}

			// Copy the textures across to the texture export folder
			for ( std::list< Data::TextureInstance* >::iterator i = usedTextures.begin(); i != usedTextures.end(); ++i )
			{
				wxString texPath = textureDir;//textureDir;
				wxString texBase = (*i)->filename_.c_str();
				texBase.Replace( "\\", "/", true );
				texPath += texBase.AfterLast( '/' );

				if ( !CopyFile( texBase.wc_str(), texPath.wc_str(), FALSE ) )
				{
					wxString msgStr;
					msgStr.Printf( "Failed to export texture \"%s\"(Error Code %d)", texBase.c_str(), GetLastError() );
					wxMessageDialog msg( NULL, msgStr );
					msg.ShowModal();
					return;
				}
			}
		}

		wxMessageDialog msg( NULL, _T( "Export Complete" ) );
		msg.ShowModal();

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ExportScene::ExportMeshList( std::list< Data::Mesh* >& usedMeshes, const wxString& meshDir, const wxString& materialDir, std::list< Data::Material* >& usedMaterials )
	{
		//Export meshes
		for ( std::list< Data::Mesh* >::iterator i = usedMeshes.begin(); i != usedMeshes.end(); ++i )
		{
			//Cache the mesh
			Data::Mesh* pMesh = *i;
			//build the full path name
			wxString meshFullPath;
			meshFullPath.Printf( "%smesh_%03d.xml", meshDir, pMesh->GetMeshSceneIndex() );

			TiXmlDocument doc; 
			doc.SetCondenseWhiteSpace( false );
			TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "UTF-8", "yes" ); 
			TiXmlElement* pMeshRootEle = new TiXmlElement( "mesh" );
			doc.LinkEndChild( decl );
			doc.LinkEndChild( pMeshRootEle );

			if ( pScene_->GetMaterial( pMesh->GetMeshMaterialIndex() ) )
			{
				Data::Material* pMat = pScene_->GetMaterial( pMesh->GetMeshMaterialIndex() );
				TiXmlElement* pMaterialEle = new TiXmlElement( "material" );
				char tmpStrBuf[256];

				sprintf( tmpStrBuf, "../materials/material_%03d.xml", pMat->GetMaterialSceneIndex() );
				TiXmlText* pMaterialText = new TiXmlText( tmpStrBuf );

				pMaterialEle->LinkEndChild( pMaterialText );
				pMeshRootEle->LinkEndChild( pMaterialEle );

				// add the found material to the list for export later,
				// assuming its not in the list already
				if ( std::find( usedMaterials.begin(), usedMaterials.end(), pMat ) == usedMaterials.end() )
				{
					usedMaterials.push_back( pMat );
				}
			}

			{
				//Build an element of the indices we use
				TiXmlElement* pIndicesEle = new TiXmlElement( "indices" );

				u32 nFaces = pMesh->GetnFaces();
				Data::Face* pFaces = pMesh->GetpFaces();

				pIndicesEle->SetAttribute( "count", nFaces );

				for ( u32 i = 0, vi = 0; i < nFaces; ++i )
				{
					//Add an element for each face
					char faceIdxStr[256];
					sprintf( faceIdxStr, "%d %d %d", pFaces[i].indices_[0], pFaces[i].indices_[1], pFaces[i].indices_[2] );
					TiXmlElement* pFaceEle = new TiXmlElement( "face" );
					TiXmlText* pIndices = new TiXmlText( faceIdxStr );
					//Add to parent
					pFaceEle->LinkEndChild( pIndices );
					pIndicesEle->LinkEndChild( pFaceEle );
				}

				//Link this to the main document
				pMeshRootEle->LinkEndChild( pIndicesEle );
			}

			{
				//Build an element of the vertices we use
				TiXmlElement* pVerticesEle = new TiXmlElement( "vertices" );

				u32 nVertices = pMesh->GetnVertices();
				Vec3* pVerts = pMesh->GetpVertices();

				pVerticesEle->SetAttribute( "count", nVertices );
				pVerticesEle->SetAttribute( "order", "xyz" );

				for ( u32 i = 0; i < nVertices; ++i )
				{
					//Add an element for each vertex
					char faceIdxStr[256];
					sprintf( faceIdxStr, "%f %f %f", pVerts[i].x, pVerts[i].y, pVerts[i].z );
					TiXmlElement* pVertEle = new TiXmlElement( "vertex" );
					TiXmlText* pVertex = new TiXmlText( faceIdxStr );
					//Add to parent
					pVertEle->LinkEndChild( pVertex );
					pVerticesEle->LinkEndChild( pVertEle );
				}

				// Link this to the main document
				pMeshRootEle->LinkEndChild( pVerticesEle );

			}

			//Do it again, Sam....
			{
				//Build an element of the normals we use
				TiXmlElement* pNormalsEle = new TiXmlElement( "normals" );

				u32 nNormals = pMesh->GetnNormals();
				Vec3* pNorms = pMesh->GetpNormals();

				pNormalsEle->SetAttribute( "count", nNormals );
				pNormalsEle->SetAttribute( "order", "xyz" );

				for ( u32 i = 0; i < nNormals; ++i )
				{
					//Add an element for each vertex
					char faceIdxStr[256];
					sprintf( faceIdxStr, "%f %f %f", pNorms[i].x, pNorms[i].y, pNorms[i].z );
					TiXmlElement* pNrmEle = new TiXmlElement( "vertex" );
					TiXmlText* pNormal = new TiXmlText( faceIdxStr );
					//Add to parent
					pNrmEle->LinkEndChild( pNormal );
					pNormalsEle->LinkEndChild( pNrmEle );
				}

				// Link this to the main document
				pMeshRootEle->LinkEndChild( pNormalsEle );
			}

			//Do it again, Sam....
			{
				//Build an element of the normals we use
				TiXmlElement* pTangentsEle = new TiXmlElement( "tangents" );

				u32 nTangents = pMesh->GetnTangents();
				Vec3* pTans = pMesh->GetpTangents();

				pTangentsEle->SetAttribute( "count", nTangents );
				pTangentsEle->SetAttribute( "order", "xyz" );

				for ( u32 i = 0; i < nTangents; ++i )
				{
					//Add an element for each vertex
					char faceIdxStr[256];
					//sanity checks...
					if ( pTans[i].x != pTans[i].x || pTans[i].y != pTans[i].y || pTans[i].z != pTans[i].z )//NAN in output?
					{
						sprintf( faceIdxStr, "%f %f %f", 0.0f, 0.0f, 1.0f );
					}
					else if ( pTans[i].x > 2.0f || pTans[i].y > 2.0f || pTans[i].z > 2.0f )// Big numbers?
					{
						sprintf( faceIdxStr, "%f %f %f", min( pTans[i].x, 2.0f ), min( pTans[i].y, 2.0f ), min( pTans[i].z, 2.0f ) );
					}
					else
					{
						sprintf( faceIdxStr, "%f %f %f", pTans[i].x, pTans[i].y, pTans[i].z );
					}
					TiXmlElement* pTanEle = new TiXmlElement( "vertex" );
					TiXmlText* pTanget = new TiXmlText( faceIdxStr );
					//Add to parent
					pTanEle->LinkEndChild( pTanget );
					pTangentsEle->LinkEndChild( pTanEle );
				}

				// Link this to the main document
				pMeshRootEle->LinkEndChild( pTangentsEle );
			}

			//Do it again, Sam....
			{
				//Build an element of the normals we use
				TiXmlElement* pBiTangentsEle = new TiXmlElement( "bitangents" );

				u32 nBiTans = pMesh->GetnBinormals();
				Vec3* pBiTans = pMesh->GetpBinormals();

				pBiTangentsEle->SetAttribute( "count", nBiTans );
				pBiTangentsEle->SetAttribute( "order", "xyz" );

				for ( u32 i = 0; i < nBiTans; ++i )
				{
					//Add an element for each vertex
					char faceIdxStr[256];
					//sanity checks...
					if ( pBiTans[i].x != pBiTans[i].x || pBiTans[i].y != pBiTans[i].y || pBiTans[i].z != pBiTans[i].z )//NAN in output?
					{
						sprintf( faceIdxStr, "%f %f %f", 0.0f, 0.0f, 1.0f );
					}
					else if ( pBiTans[i].x > 2.0f || pBiTans[i].y > 2.0f || pBiTans[i].z > 2.0f )// Big numbers?
					{
						sprintf( faceIdxStr, "%f %f %f", min( pBiTans[i].x, 2.0f ), min( pBiTans[i].y, 2.0f ), min( pBiTans[i].z, 2.0f ) );
					}
					else
					{
						sprintf( faceIdxStr, "%f %f %f", pBiTans[i].x, pBiTans[i].y, pBiTans[i].z );
					}
					TiXmlElement* pBiTanEle = new TiXmlElement( "vertex" );
					TiXmlText* pBiTangent = new TiXmlText( faceIdxStr );
					//Add to parent
					pBiTanEle->LinkEndChild( pBiTangent );
					pBiTangentsEle->LinkEndChild( pBiTanEle );
				}

				// Link this to the main document
				pMeshRootEle->LinkEndChild( pBiTangentsEle );
			}

			//
			{
				for ( u32 i = 0; i < pMesh->GetnUVChannels(); ++i )
				{
					//Build an element of the normals we use
					TiXmlElement* pUVsEle = new TiXmlElement( "UV" );

					u32 nUVElements = pMesh->GetUVChannelsElementCount( i );
					Vec3* pUVs = pMesh->GetpUVChannel( i );
					u32 nUVs = pMesh->GetUVChannelCount( i );

					pUVsEle->SetAttribute( "channel", i );
					pUVsEle->SetAttribute( "elementcount", nUVElements );
					pUVsEle->SetAttribute( "count", nUVs );


					const char* formatStr = NULL;
					switch ( nUVElements )
					{
					case 1: formatStr = "%f"; break;
					case 2: formatStr = "%f %f"; break;
					case 3: formatStr = "%f %f %f"; break;
					}
					for ( u32 i2 = 0; i2 < nUVs; ++i2 )
					{
						char tmpBuf[32];
						sprintf( tmpBuf, formatStr, pUVs[i2].x, 1.0f-pUVs[i2].y, pUVs[i2].z );
						TiXmlElement* pUVChanEle = new TiXmlElement( "vertex" );
						TiXmlText* pUVChan = new TiXmlText( tmpBuf );
						//Add to parent
						pUVChanEle->LinkEndChild( pUVChan );
						pUVsEle->LinkEndChild( pUVChanEle );
					}

					// Link this to the main document
					pMeshRootEle->LinkEndChild( pUVsEle );
				}
			}

			// Do Colour info
			{
				for ( u32 i = 0; i < pMesh->GetnColourChannels(); ++i )
				{
					//Build an element of the normals we use
					TiXmlElement* pColourChanelEle = new TiXmlElement( "UV" );
					u32 nColourEles = pMesh->GetColourChannelCount( i );
					Vec4* pColorData = pMesh->GetpColourChannel( i );

					pColourChanelEle->SetAttribute( "channel", i );
					pColourChanelEle->SetAttribute( "elementcount", nColourEles );
					pColourChanelEle->SetAttribute( "order", "rgba" );

					for ( u32 i2 = 0; i2 < nColourEles; ++i2 )
					{
						char tmpBuf[32];
						sprintf( tmpBuf, "%f %f %f %f", pColorData[i].x, pColorData[i].y, pColorData[i].z, pColorData[i].w );
						TiXmlElement* pColourEle = new TiXmlElement( "colour" );
						TiXmlText* pColourTxt = new TiXmlText( tmpBuf );
						//Add to parent
						pColourEle->LinkEndChild( pColourTxt );
						pColourTxt->LinkEndChild( pColourEle );
					}

					// Link this to the main document
					pMeshRootEle->LinkEndChild( pColourChanelEle );
				}
			}

			//output this built xml file
			doc.SaveFile( meshFullPath.c_str() );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ExportScene::VisitScene( HScene::SceneGraph* pScene, TiXmlElement* pRoot, std::list< Data::Mesh* >* pMeshList )
	{
		struct NodeLevel
		{
			HScene::Node* pnode;
			TiXmlElement* pxmlelement;
		};
		std::stack< NodeLevel > visitStack;

		NodeLevel first = { pScene->pRoot(), pRoot };
		visitStack.push( first );

		while ( !visitStack.empty() ) 
		{
			//Get this levels info
			NodeLevel pnodelevel = visitStack.top();
			visitStack.pop();
			HScene::Node* pnode = pnodelevel.pnode;
			TiXmlElement* pxmlele = pnodelevel.pxmlelement;

			//Create Node Element
			TiXmlElement* pnodeelement = new TiXmlElement( "node" );
			pnodeelement->SetAttribute( "type", pnode->NodeTypeName() );

			TiXmlElement* pnameele = new TiXmlElement( "name" );
			TiXmlText* pname = new TiXmlText( pnode->pName() );
			pnameele->LinkEndChild( pname );
			pnodeelement->LinkEndChild( pnameele );

			TiXmlElement* pmatrixele = new TiXmlElement( "matrix" );
			wxString matrixstr;
			matrixstr.Printf( "(%f,%f,%f,%f)(%f,%f,%f,%f)(%f,%f,%f,%f)(%f,%f,%f,%f)",
				pnode->matrix_._11, pnode->matrix_._12, pnode->matrix_._13, pnode->matrix_._14,
				pnode->matrix_._21, pnode->matrix_._22, pnode->matrix_._23, pnode->matrix_._24,
				pnode->matrix_._31, pnode->matrix_._32, pnode->matrix_._33, pnode->matrix_._34,
				pnode->matrix_._41, pnode->matrix_._42, pnode->matrix_._43, pnode->matrix_._44 );
			TiXmlText* pmatrix = new TiXmlText( matrixstr.c_str() );
			pmatrixele->LinkEndChild( pmatrix );
			pnodeelement->LinkEndChild( pmatrixele );

			TiXmlElement* pgmatrixele = new TiXmlElement( "globalmatrix" );
			wxString gmatrixstr;
			gmatrixstr.Printf( "(%f,%f,%f,%f)(%f,%f,%f,%f)(%f,%f,%f,%f)(%f,%f,%f,%f)",
				pnode->globalMatrix_._11, pnode->globalMatrix_._12, pnode->globalMatrix_._13, pnode->globalMatrix_._14,
				pnode->globalMatrix_._21, pnode->globalMatrix_._22, pnode->globalMatrix_._23, pnode->globalMatrix_._24,
				pnode->globalMatrix_._31, pnode->globalMatrix_._32, pnode->globalMatrix_._33, pnode->globalMatrix_._34,
				pnode->globalMatrix_._41, pnode->globalMatrix_._42, pnode->globalMatrix_._43, pnode->globalMatrix_._44 );
			TiXmlText* pgmatrix = new TiXmlText( gmatrixstr.c_str() );
			pgmatrixele->LinkEndChild( pgmatrix );
			pnodeelement->LinkEndChild( pgmatrixele );

			XML_LINK_INT_AS_STRING( "castshadow", pnode->GetCastShadows(), pnodeelement );

			if ( strcmp( pnode->NodeTypeName(), HScene::MeshCollection::StaticNodeTypeName ) == 0 )
			{
				TiXmlElement* pmeshesele = new TiXmlElement( "meshes" );
				HScene::MeshCollection* ptnode = (HScene::MeshCollection*)pnode;
				for ( u32 i = 0; i < ptnode->nMeshes(); ++i )
				{
					Data::Mesh* pmesh = ptnode->pMesh(i);
					wxString meshid;
					meshid.Printf( "%d", pmesh->GetMeshSceneIndex() );

					//Add to the found mesh list
					if ( std::find( pMeshList->begin(), pMeshList->end(), pmesh ) == pMeshList->end() )
					{
						pMeshList->push_back( pmesh );
					}
					
					//Create a mesh id node
					TiXmlElement* pmeshmeshidele = new TiXmlElement( "meshindex" );
					TiXmlText* pmeshmeshid = new TiXmlText( meshid.c_str() );
					pmeshmeshidele->LinkEndChild( pmeshmeshid );

					//Add scene id & mesh id to mesh element
					TiXmlElement* pmeshele = new TiXmlElement( "mesh" );
					pmeshele->LinkEndChild( pmeshmeshidele );

					pmeshesele->LinkEndChild( pmeshele );
				}

				pnodeelement->LinkEndChild( pmeshesele );
			}
			else if ( strcmp( pnode->NodeTypeName(), HScene::Light::StaticNodeTypeName ) == 0 )
			{
				TiXmlElement* plightele = new TiXmlElement( "lightdata" );
				HScene::Light* plight = (HScene::Light*)pnode;

				// Light Type
				wxString typeStr;
				switch( plight->lightType_ )
				{
				case HScene::LightType_Direction:
					typeStr = "LIGHTSOURCETYPE_DIRECTION";
					break;
				case HScene::LightType_Point:
					typeStr = "LIGHTSOURCETYPE_POINT";
					break;
				case HScene::LightType_Spot:
					typeStr = "LIGHTSOURCETYPE_SPOT";
					break;
				}
				TiXmlElement* ptypeEle = new TiXmlElement( "type" );
				TiXmlText* ptype = new TiXmlText( typeStr.c_str() );
				ptypeEle->LinkEndChild( ptype );
				plightele->LinkEndChild( ptypeEle );

				// position
				wxString posStr;
				posStr.Printf( "%f,%f,%f", plight->matrix_._41, plight->matrix_._42, plight->matrix_._43 );
				TiXmlElement* pposEle = new TiXmlElement( "position" );
				TiXmlText* ppos = new TiXmlText( posStr );
				pposEle->LinkEndChild( ppos );
				plightele->LinkEndChild( pposEle );

				// light direction
				wxString dirStr;
				dirStr.Printf( "%f,%f,%f", plight->direction_.x, plight->direction_.y, plight->direction_.z );
				TiXmlElement* pdirEle = new TiXmlElement( "direction" );
				TiXmlText* pdir = new TiXmlText( dirStr.c_str() );
				pdirEle->LinkEndChild( pdir );
				plightele->LinkEndChild( pdirEle );

				// attenuation
				TiXmlElement* pconstAttEle = new TiXmlElement( "attenuation" );
				pconstAttEle->SetDoubleAttribute( "constant", plight->attenuation_ );
				pconstAttEle->SetDoubleAttribute( "linear", plight->attenuationLinear_ );
				pconstAttEle->SetDoubleAttribute( "quadratic", plight->attenuationQuadratic_ );
				plightele->LinkEndChild( pconstAttEle );

				// light Colours
				// Diffuse
				TiXmlElement* pdiffuseEle = new TiXmlElement( "diffuse" );
				pdiffuseEle->SetDoubleAttribute( "red", plight->diffuse_.r );
				pdiffuseEle->SetDoubleAttribute( "green", plight->diffuse_.g );
				pdiffuseEle->SetDoubleAttribute( "blue", plight->diffuse_.b );
				pdiffuseEle->SetDoubleAttribute( "alpha", plight->diffuse_.a );
				plightele->LinkEndChild( pdiffuseEle );

				// Specular
				TiXmlElement* pspecEle = new TiXmlElement( "specular" );
				pspecEle->SetDoubleAttribute( "red", plight->specular_.r );
				pspecEle->SetDoubleAttribute( "green", plight->specular_.g );
				pspecEle->SetDoubleAttribute( "blue", plight->specular_.b );
				pspecEle->SetDoubleAttribute( "alpha", plight->specular_.a );
				plightele->LinkEndChild( pspecEle );

				// Ambient
				TiXmlElement* pamibentEle = new TiXmlElement( "ambient" );
				pamibentEle->SetDoubleAttribute( "red", plight->ambient_.r );
				pamibentEle->SetDoubleAttribute( "green", plight->ambient_.g );
				pamibentEle->SetDoubleAttribute( "blue", plight->ambient_.b );
				pamibentEle->SetDoubleAttribute( "alpha", plight->ambient_.a );
				plightele->LinkEndChild( pamibentEle );

				// light Cone
				TiXmlElement* pconeEle = new TiXmlElement( "lightcone" );
				pconeEle->SetDoubleAttribute( "innerangle", plight->innerCone_ );
				pconeEle->SetDoubleAttribute( "outerangle", plight->outerCone_ );
				plightele->LinkEndChild( pconeEle );

				// light power
				wxString powerStr;
				powerStr.Printf( "%f", plight->brightness_ );
				TiXmlElement* ppowerEle = new TiXmlElement( "lightpower" );
				TiXmlText* ppower = new TiXmlText( powerStr );
				ppowerEle->LinkEndChild( ppower );
				plightele->LinkEndChild( ppowerEle );

				// light min R
				wxString radStr;
				radStr.Printf( "%f", plight->minRadius_ );
				TiXmlElement* pradminEle = new TiXmlElement( "minradius" );
				TiXmlText* pradmin = new TiXmlText( radStr );
				pradminEle->LinkEndChild( pradmin );
				plightele->LinkEndChild( pradminEle );

				// light max R
				radStr.Printf( "%f", plight->maxRadius_ );
				TiXmlElement* pradmaxEle = new TiXmlElement( "maxradius" );
				TiXmlText* pradmax = new TiXmlText( radStr );
				pradmaxEle->LinkEndChild( pradmax );
				plightele->LinkEndChild( pradmaxEle );

				// light falloff
				wxString foStr;
				foStr.Printf( "%f", plight->falloff_ );
				TiXmlElement* pfoEle = new TiXmlElement( "falloff" );
				TiXmlText* pfo = new TiXmlText( foStr );
				pfoEle->LinkEndChild( pfo );
				plightele->LinkEndChild( pfoEle );

				// Finally Add the build light node data to the node element
				pnodeelement->LinkEndChild( plightele );

			}
			else if ( strcmp( pnode->NodeTypeName(), HScene::Camera::StaticNodeTypeName ) == 0 )
			{
				TiXmlElement* pcamEle = new TiXmlElement( "cameradata" );
				HScene::Camera* pcam = (HScene::Camera*)pnode;

				// comment attributes
				pcamEle->SetDoubleAttribute( "fov", pcam->fov_ );
				pcamEle->SetDoubleAttribute( "near", pcam->nearPlane_ );
				pcamEle->SetDoubleAttribute( "far", pcam->farPlane_ );
				pcamEle->SetDoubleAttribute( "aspect", pcam->aspect_ );

				// Name element 
				TiXmlElement* pnameEle = new TiXmlElement( "name" );
				TiXmlText* pname = new TiXmlText( pcam->pName() );
				pnameEle->LinkEndChild( pname );
				pcamEle->LinkEndChild( pnameEle );

				// camera position
				wxString posStr;
				posStr.Printf( "%f,%f,%f", pcam->matrix_._41, pcam->matrix_._42, pcam->matrix_._43 );
				TiXmlElement* pposEle = new TiXmlElement( "position" );
				TiXmlText* ppos = new TiXmlText( posStr.c_str() );
				pposEle->LinkEndChild( ppos );
				pcamEle->LinkEndChild( pposEle );

				// camera look at 
				wxString atStr;
				atStr.Printf( "%f,%f,%f", pcam->matrix_._13, pcam->matrix_._23, pcam->matrix_._33 );
				TiXmlElement* patEle = new TiXmlElement( "at" );
				TiXmlText* pat = new TiXmlText( atStr.c_str() );
				patEle->LinkEndChild( pat );
				pcamEle->LinkEndChild( patEle );

				// camera up vector
				wxString upStr;
				upStr.Printf( "%f,%f,%f", pcam->matrix_._12, pcam->matrix_._22, pcam->matrix_._32 );
				TiXmlElement* pupEle = new TiXmlElement( "up" );
				TiXmlText* pup = new TiXmlText( upStr.c_str() );
				pupEle->LinkEndChild( pup );
				pcamEle->LinkEndChild( pupEle );

				// Finally, link the camera element to the main node element
				pnodeelement->LinkEndChild( pcamEle );
			}

			for ( HScene::Node::ChildListType::const_reverse_iterator i = pnode->pChildren()->rbegin(); i != pnode->pChildren()->rend(); ++i )
			{
				NodeLevel child = { *i, pnodeelement };
				visitStack.push( child );
			}

			//Done. Link this new element with parent
			pxmlele->LinkEndChild( pnodeelement );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ExportScene::Undo()
	{

	}

}