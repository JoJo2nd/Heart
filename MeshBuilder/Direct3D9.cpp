/********************************************************************

	filename: 	Direct3D9.cpp	
	
	Copyright (c) 15:1:2011 James Moran
	
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
#include "Direct3D9.h"
#include "DataMaterial.h"
#include "SceneDatabase.h"
#include "resource.h"
#include "Basic.h"
#include "wx\wx.h"
#include "wx\dc.h"

namespace Direct3D
{
namespace 
{
	LPDIRECT3D9 gpD3D = NULL;
}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool InitD3D()
	{
		if ( !gpD3D )
		{
			gpD3D = Direct3DCreate9( D3D_SDK_VERSION );
		}
		return gpD3D != NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void FinaliseD3D()
	{
		if ( gpD3D )
		{
			gpD3D->Release();
			gpD3D = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool CreateD3DDevice( HWND hWnd, Device* pOutDevice )
	{
		if ( !InitD3D() )
		{
			return false;
		}

		GetClientRect( hWnd, &pOutDevice->displayRect_ );

		D3DPRESENT_PARAMETERS* parameters = &pOutDevice->presentParameters_;
		memset( parameters, 0, sizeof( D3DPRESENT_PARAMETERS ) );

		D3DDISPLAYMODE displayMode;
		gpD3D->GetAdapterDisplayMode( 0, &displayMode );

		parameters->Windowed = TRUE;
		parameters->hDeviceWindow = hWnd;
		parameters->EnableAutoDepthStencil = TRUE;
		parameters->PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		parameters->BackBufferWidth = 1024;
		parameters->BackBufferHeight = 1024;
		parameters->SwapEffect = D3DSWAPEFFECT_DISCARD;
		parameters->BackBufferCount = 1;

		// check whether we can use a D32 depth buffer format
		if ( SUCCEEDED(gpD3D->CheckDeviceFormat( 0, D3DDEVTYPE_HAL, displayMode.Format, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D32 )) &&
			 SUCCEEDED(gpD3D->CheckDepthStencilMatch( 0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_D32 )) )
		{
			parameters->AutoDepthStencilFormat = D3DFMT_D32;
		}
		else
		{
			parameters->AutoDepthStencilFormat = D3DFMT_D24X8;
		}

		// find the highest multi-sample type available on this device
		D3DMULTISAMPLE_TYPE sMS = D3DMULTISAMPLE_2_SAMPLES;
		D3DMULTISAMPLE_TYPE sMSOut = D3DMULTISAMPLE_NONE;
		DWORD dwQuality = 0;
		while ((D3DMULTISAMPLE_TYPE)(D3DMULTISAMPLE_16_SAMPLES + 1) != (sMS = (D3DMULTISAMPLE_TYPE)(sMS + 1)))
		{
		 if ( SUCCEEDED(gpD3D->CheckDeviceMultiSampleType( 0, D3DDEVTYPE_HAL, displayMode.Format, TRUE, sMS, &dwQuality )) )
		 {
			 sMSOut = sMS;
		 }
		}
		if ( dwQuality != 0 )
		{
			dwQuality -= 1;
		}

		parameters->MultiSampleQuality = dwQuality;
		parameters->MultiSampleType = sMSOut;

		//check if we can use hardware vertex processing
		D3DV(gpD3D->GetDeviceCaps( 0, D3DDEVTYPE_HAL, &pOutDevice->caps_ ));
		pOutDevice->creationFlags_ = 0;
		if ( pOutDevice->caps_.VertexShaderVersion >= D3DVS_VERSION( 2, 0 ) )
		{
			pOutDevice->creationFlags_ |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
		}
		else
		{
			pOutDevice->creationFlags_ |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		}

		//Create the device
		if ( FAILED(gpD3D->CreateDevice( 
			0, 
			D3DDEVTYPE_HAL, 
			hWnd, 
			pOutDevice->creationFlags_, 
			&pOutDevice->presentParameters_, 
			&pOutDevice->pD3DDevice_ )) )
		{
			return false;
		}

		D3DV(pOutDevice->pD3DDevice_->SetRenderState( D3DRS_DITHERENABLE, TRUE ));

		if ( !pOutDevice->CreateResources() )
		{
			return false;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool ResetD3DDevice( Device* pOutDevice )
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool DestroyD3DDevice( Device* pOutDevice )
	{
		if ( pOutDevice && pOutDevice->pD3DDevice_ )
		{
			pOutDevice->pD3DDevice_->Release();
			pOutDevice->pD3DDevice_ = NULL;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool Device::CreateResources()
	{
		D3DV( D3DXCreateTextureFromResource( pD3DDevice_, GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG1), &pProblemTex_ ) );
		D3DV( D3DXCreateTextureFromResource( pD3DDevice_, GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG2), &pCamTex_ ) );
		D3DV( D3DXCreateTextureFromResource( pD3DDevice_, GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PNG3), &pLightTex_ ) );

		D3DV( D3DXCreateSphere( pD3DDevice_, 1.0f, 16, 16, &sphereMesh_, NULL ) );
		D3DV( D3DXCreateCylinder( pD3DDevice_, 0.0f, 1.0f, 1.0f, 8, 1, &coneMesh_, NULL ) );

		D3DV( D3DXCreateSprite( pD3DDevice_, &pSpriteRender_ ) );

		D3DV( D3DXCreateEffect( pD3DDevice_, Basic_data_start, sizeof(Basic_data_start), NULL, NULL, D3DXSHADER_DEBUG, NULL, &pRenderEffect_, NULL ) );
		if ( pRenderEffect_ )
		{
			for ( u32 i = 0; pRenderEffect_->GetParameter( NULL, i ) != NULL; ++i )
			{
				D3DXHANDLE hParam = pRenderEffect_->GetParameter( NULL, i );
				D3DXPARAMETER_DESC desc;
				pRenderEffect_->GetParameterDesc( hParam, &desc );

				if ( desc.Semantic && strcmp( desc.Semantic, "DIFFUSE0" ) == 0 )
				{
					diffuseTextureParam_ = hParam;
				}
				else if ( desc.Semantic && strcmp( desc.Semantic, "WORLDVIEWPROJECTION" ) == 0 )
				{
					worldviewprojParam_ = hParam;
				}
			}
		}

		D3DXVECTOR3 eye( 0.0f, 0.0f, -1.0f );
		D3DXVECTOR3 at( 0.0f, 0.0f, 0.0f );
		D3DXVECTOR3 up( 0.0f, 1.0f, 0.0f );
		viewAt_ = at;
		viewEye_ = eye;
		D3DXMatrixIdentity( &viewRot_ );
		D3DXMatrixIdentity( &view_ );
		view_._43 = -3.0f;
		D3DXMatrixPerspectiveFovLH( &projection_, D3DX_PI/4.0f, 4.0f/3.0f, 0.01f, 5000.0f );

		D3DXQuaternionIdentity( &viewQuat_ );

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Device::DestroyResources()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Device::ClearTarget( D3DCOLOR colour, float z )
	{
		D3DV(pD3DDevice_->Clear( 
			0, 
			NULL, 
			D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
			colour, 
			z, 
			0 ));
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Device::BeginRender( u32 w, u32 h )
	{
		D3DV(pD3DDevice_->BeginScene());
		D3DV(pD3DDevice_->SetRenderState( D3DRS_ZENABLE, TRUE ) );
		D3DV(pD3DDevice_->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESS ) );
		D3DV(pD3DDevice_->SetRenderState( D3DRS_ZWRITEENABLE, TRUE ) );
		D3DV(pD3DDevice_->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW ) );


		D3DXMatrixPerspectiveFovLH( &projection_, D3DX_PI/4.0f, (float)w/(float)h, 0.5f, 5000.0f );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Device::EndRender()
	{
		D3DV(pD3DDevice_->EndScene());
		D3DV(pD3DDevice_->Present(NULL,NULL,NULL,NULL));
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool Device::ApplyMaterial( Data::Material* pmat, const D3DXMATRIX* pworld )
	{
		u32 hTex = pmat->GetDiffuseTextureHandle( 0 );
		Data::TextureInstance* pMatTex = pmat->GetDiffuseTexture( 0 );
		
		if ( hTex != Data::INVALIDTEXTUREHANDLE && pMatTex  )
		{
			//Find the loaded Texture
			LPDIRECT3DTEXTURE9 pTex = pProblemTex_;
			TextureMap::iterator it = loadedTextures_.find( hTex );
			if ( it == loadedTextures_.end() )
			{
				//Texture doesn't exist, so load it
				TextureData td;
				td.file_ = pMatTex->filename_;
				if ( SUCCEEDED( D3DXCreateTextureFromFileA( pD3DDevice_, td.file_.c_str(), &td.pTex_ ) ) )
				{
					//Add to the list of loaded textures
					loadedTextures_[hTex] = td;
					it = loadedTextures_.find( hTex );
				}

			}
			else if ( it->second.file_ != pMatTex->filename_ )
			{
				//Check the texture isn't different from last load
				LPDIRECT3DTEXTURE9 pPrev = it->second.pTex_;
				if ( SUCCEEDED( D3DXCreateTextureFromFileA( pD3DDevice_, pMatTex->filename_.c_str(), &it->second.pTex_ ) ) )
				{
					it->second.file_ = pMatTex->filename_;
					pTex = it->second.pTex_;
					pPrev->Release();
				}
			}
			else if ( it != loadedTextures_.end() )
			{
				pTex = it->second.pTex_;
			}

			pRenderEffect_->SetTexture( diffuseTextureParam_, pTex );
		}
		else 
		{
			pRenderEffect_->SetTexture( diffuseTextureParam_, pProblemTex_ );
		}

 		D3DXMATRIX wvp;
		D3DXMATRIX view;

		BuildWVPMatrix( pworld, &view, &wvp );
 
 		pRenderEffect_->SetMatrix( worldviewprojParam_, &wvp );

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Device::DrawIndexedPrimative( Data::Material* pmat, const D3DXMATRIX* pworld, u32 vertexFlags, u32 indexcound, u32 vertexcount, u32 vertexstride, void* indices, void* vertices )
	{
		D3DXHANDLE hTechPosNormalUV = pRenderEffect_->GetTechniqueByName( "MainPosNormalUV" );
		D3DXHANDLE hTechPosNormal = pRenderEffect_->GetTechniqueByName( "MainPosNormal" );
		D3DXHANDLE hTechPos = pRenderEffect_->GetTechniqueByName( "MainPos" );

		if ( hTechPosNormalUV && hTechPosNormal &&  hTechPos )
		{
			UINT passes;

			SetVertexDeclaration( vertexFlags );

			if ( (vertexFlags & (VF_POSITION | VF_NORMAL | VF_UV1)) == (VF_POSITION | VF_NORMAL | VF_UV1) )
			{
				pRenderEffect_->SetTechnique( hTechPosNormalUV );
			}
			else if ( (vertexFlags & (VF_POSITION | VF_NORMAL )) == (VF_POSITION | VF_NORMAL ) )
			{
				pRenderEffect_->SetTechnique( hTechPosNormal );
			}
			else if ( (vertexFlags & (VF_POSITION)) == VF_POSITION )
			{
				pRenderEffect_->SetTechnique( hTechPos );
			}

			ApplyMaterial( pmat, pworld );
			pRenderEffect_->Begin( &passes, 0 );
			for ( UINT i = 0; i < passes; ++i )
			{
				pRenderEffect_->BeginPass( i );
				D3DV( pD3DDevice_->DrawIndexedPrimitiveUP( D3DPT_TRIANGLELIST, 0, vertexcount, indexcound/3, indices, D3DFMT_INDEX16, vertices, vertexstride ) );
				pRenderEffect_->EndPass();
			}
			pRenderEffect_->End();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Device::DrawLightPrimative( const D3DXMATRIX* pworld )
	{
		D3DXMATRIX wvp;
		D3DXMATRIX view;
		D3DXVECTOR3 cen( pLightTex_->Width/2.0f, pLightTex_->Height/2.0f, 0.0f );

		BuildWVPMatrix( pworld, &view, &wvp );

		//pSpriteRender_->SetTransform( pworld );
		pSpriteRender_->SetWorldViewLH( pworld, &view );

		pD3DDevice_->SetTransform( D3DTS_PROJECTION, &projection_ );

		pSpriteRender_->Begin( D3DXSPRITE_BILLBOARD | D3DXSPRITE_OBJECTSPACE | D3DXSPRITE_ALPHABLEND );
		pSpriteRender_->Draw( pLightTex_, NULL, &cen, NULL, D3DCOLOR_RGBA( 255, 255, 255, 255 ) );
		pSpriteRender_->End();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Device::DrawSpherePrimative( const D3DXMATRIX* world, float radius )
	{
		D3DXMATRIX wvp;
		D3DXMATRIX scale;
		D3DXMATRIX view;

		D3DXMatrixScaling( &scale, radius, radius, radius );
		BuildWVPMatrix( world, &view, &wvp );

		D3DXHANDLE hTechPosNormal = pRenderEffect_->GetTechniqueByName( "MainPosNormal" );
		D3DXHANDLE hTechPos = pRenderEffect_->GetTechniqueByName( "MainPos" );
		UINT passes;

		SetVertexDeclaration( VF_POSITION | VF_NORMAL );

		pD3DDevice_->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

		wvp = scale * wvp;

		pRenderEffect_->SetMatrix( worldviewprojParam_, &wvp );

		pRenderEffect_->SetTechnique( hTechPosNormal );
		pRenderEffect_->Begin( &passes, 0 );
		for ( UINT i = 0; i < passes; ++i )
		{
			pRenderEffect_->BeginPass( i );
			sphereMesh_->DrawSubset( 0 );
			pRenderEffect_->EndPass();
		}
		pRenderEffect_->End();

		pD3DDevice_->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Device::DrawConePrimative( const D3DXMATRIX* world, float lenght, float angle )
	{
		D3DXMATRIX wvp;
		D3DXMATRIX scale;
		D3DXMATRIX view;

		D3DXMatrixScaling( &scale, tan(angle)*lenght, tan(angle)*lenght, lenght );
		scale._43 = lenght / 2.0f;
		BuildWVPMatrix( world, &view, &wvp );

		D3DXHANDLE hTechPosNormal = pRenderEffect_->GetTechniqueByName( "MainPosNormal" );
		D3DXHANDLE hTechPos = pRenderEffect_->GetTechniqueByName( "MainPos" );
		UINT passes;

		SetVertexDeclaration( VF_POSITION | VF_NORMAL );

		pD3DDevice_->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

		wvp = scale * wvp;

		pRenderEffect_->SetMatrix( worldviewprojParam_, &wvp );

		pRenderEffect_->SetTechnique( hTechPosNormal );
		pRenderEffect_->Begin( &passes, 0 );
		for ( UINT i = 0; i < passes; ++i )
		{
			pRenderEffect_->BeginPass( i );
			coneMesh_->DrawSubset( 0 );
			pRenderEffect_->EndPass();
		}
		pRenderEffect_->End();

		pD3DDevice_->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void Device::SetViewTranslation( const D3DXVECTOR3 t )
	{
		viewAt_ = t;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Device::DrawCameraPrimative( const D3DXMATRIX* pworld )
	{
		D3DXMATRIX wvp;
		D3DXMATRIX view;
		D3DXVECTOR3 cen( pCamTex_->Width/2.0f, pCamTex_->Height/2.0f, 0.0f );

		BuildWVPMatrix( pworld, &view, &wvp );


		//pSpriteRender_->SetTransform( pworld );
		pSpriteRender_->SetWorldViewLH( pworld, &view );

		pD3DDevice_->SetTransform( D3DTS_PROJECTION, &projection_ );

		pSpriteRender_->Begin( D3DXSPRITE_BILLBOARD | D3DXSPRITE_OBJECTSPACE | D3DXSPRITE_ALPHABLEND );
		pSpriteRender_->Draw( pCamTex_, NULL, &cen, NULL, D3DCOLOR_RGBA( 255, 255, 255, 255 ) );
		pSpriteRender_->End();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Device::DrawLocatorPrimative( const D3DXMATRIX* pworld )
	{
		D3DXHANDLE hTechPos = pRenderEffect_->GetTechniqueByName( "MainLine" );

		SetVertexDeclaration( VF_POSITION | VF_COLOUR1 );


		struct Vec
		{
			D3DXVECTOR3 v;
			DWORD c;
		};
		u32 vertexstride = sizeof( Vec );
		Vec vertices[] = 
		{
			{D3DXVECTOR3(-1.0f,0.0f,0.0f),D3DCOLOR_RGBA(255,0,0,255)},{D3DXVECTOR3(1.0f,0.0f,0.0f),D3DCOLOR_RGBA(255,0,0,255)},
			{D3DXVECTOR3(0.0f,-1.0f,0.0f),D3DCOLOR_RGBA(0,255,0,255)},{D3DXVECTOR3(0.0f,1.0f,0.0f),D3DCOLOR_RGBA(0,255,0,255)},
			{D3DXVECTOR3(0.0f,0.0f,-1.0f),D3DCOLOR_RGBA(0,0,255,255)},{D3DXVECTOR3(0.0f,0.0f,1.0f),D3DCOLOR_RGBA(0,0,255,255)},
		};

		pRenderEffect_->SetTechnique( hTechPos );

		D3DXMATRIX wvp;
		D3DXMATRIX view;

		BuildWVPMatrix( pworld, &view, &wvp );

		pRenderEffect_->SetMatrix( worldviewprojParam_, &wvp );

		pD3DDevice_->SetRenderState( D3DRS_ANTIALIASEDLINEENABLE, TRUE );

		UINT passes;
		D3DV( pRenderEffect_->Begin( &passes, 0 ) );
		for ( UINT i = 0; i < passes; ++i )
		{
			D3DV( pRenderEffect_->BeginPass( i ) );
			D3DV( pD3DDevice_->DrawPrimitiveUP( D3DPT_LINELIST, 3, vertices, vertexstride ) );
			D3DV( pRenderEffect_->EndPass() );
		}
		D3DV( pRenderEffect_->End() );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Device::SetVertexDeclaration( u32 vertexFlags )
	{
		VertDeclMap::iterator vd = vertxDecl_.find( vertexFlags );

		if ( vd == vertxDecl_.end() )
		{
			CreateVertexDeclaration( vertexFlags );
			vd = vertxDecl_.find( vertexFlags );
		}
		
		pD3DDevice_->SetVertexDeclaration( vd->second );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	LPDIRECT3DVERTEXDECLARATION9 Device::CreateVertexDeclaration( u32 vertexFlags )
	{
		D3DVERTEXELEMENT9 elements[ 12 ];
		D3DVERTEXELEMENT9 endmarker[] = { D3DDECL_END() };
		u32 elementsadded = 0;
		WORD offset = 0;

		// declaration doesn't exist so create it [11/23/2008 James]
		if ( vertexFlags & VF_POSITION )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_FLOAT3;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_POSITION;
			elements[ elementsadded ].UsageIndex = 0;

			++elementsadded;
			offset += sizeof( float ) * 3;
		}
		if ( vertexFlags & VF_NORMAL )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_FLOAT3;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_NORMAL;
			elements[ elementsadded ].UsageIndex = 0;

			++elementsadded;
			offset += sizeof( float ) * 3;
		}
		if ( vertexFlags & VF_TANGENT )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_FLOAT3;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_TANGENT;
			elements[ elementsadded ].UsageIndex = 0;

			++elementsadded;
			offset += sizeof( float ) * 3;
		}
		if ( vertexFlags & VF_BINORMAL )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_FLOAT3;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_BINORMAL;
			elements[ elementsadded ].UsageIndex = 0;

			++elementsadded;
			offset += sizeof( float ) * 3;
		}
		if ( vertexFlags & VF_COLOUR1 )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_D3DCOLOR;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_COLOR;
			elements[ elementsadded ].UsageIndex = 0;

			++elementsadded;
			offset += sizeof( u8 ) * 4;
		}
		if ( vertexFlags & VF_COLOUR2 )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_D3DCOLOR;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_COLOR;
			elements[ elementsadded ].UsageIndex = 1;

			++elementsadded;
			offset += sizeof( u8 ) * 4;
		}
		if ( vertexFlags & VF_COLOUR3 )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_D3DCOLOR;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_COLOR;
			elements[ elementsadded ].UsageIndex = 2;

			++elementsadded;
			offset += sizeof( u8 ) * 4;
		}
		if ( vertexFlags & VF_COLOUR4 )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_D3DCOLOR;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_COLOR;
			elements[ elementsadded ].UsageIndex = 3;

			++elementsadded;
			offset += sizeof( u8 ) * 4;
		}
		if ( vertexFlags & VF_UV1 )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_FLOAT2;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_TEXCOORD;
			elements[ elementsadded ].UsageIndex = 0;

			++elementsadded;
			offset += sizeof( float ) * 2;
		}
		if ( vertexFlags & VF_UV2 )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_FLOAT2;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_TEXCOORD;
			elements[ elementsadded ].UsageIndex = 1;

			++elementsadded;
			offset += sizeof( float ) * 2;
		}
		if ( vertexFlags & VF_UV3 )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_FLOAT2;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_TEXCOORD;
			elements[ elementsadded ].UsageIndex = 2;

			++elementsadded;
			offset += sizeof( float ) * 2;
		}
		if ( vertexFlags & VF_UV4 )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_FLOAT2;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_TEXCOORD;
			elements[ elementsadded ].UsageIndex = 3;

			++elementsadded;
			offset += sizeof( float ) * 2;
		}

		elements[ elementsadded ] = endmarker[ 0 ];

		LPDIRECT3DVERTEXDECLARATION9 pvd;
		HRESULT hr = pD3DDevice_->CreateVertexDeclaration( elements, &pvd );
		ASSERT( hr == D3D_OK );

		vertxDecl_[vertexFlags] = pvd;
		return pvd;
	}

	//////////////////////////////////////////////////////////////////////////
	// 23:19:41 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Device::RotateView( float xdegs, float ydegs )
	{
		D3DXMATRIX mat;//,matx,maty;
		D3DXQUATERNION xr;


		if ( ydegs )
		{
			D3DXVECTOR3 up( 0.0f, 1.0f, 0.0f );
			D3DXMatrixRotationAxis( &mat, &up, ( ydegs * ( D3DX_PI / 180.0f )) );

			viewRot_ = viewRot_*mat;
		}
		if ( xdegs )
		{
			D3DXVECTOR3 right( 1.0f, 0.0f, 0.0f );
			D3DXMatrixRotationAxis( &mat, &right, ( xdegs * ( D3DX_PI / 180.0f )) );

			viewRot_ = mat*viewRot_;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// 23:19:49 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Device::TranslateView( const D3DXVECTOR3* by )
	{
		D3DXVECTOR3 m;
		D3DXVec3TransformNormal( &m, by, &viewRot_ );
		viewAt_ += m;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void Device::ZoomView( float z )
	{
		const D3DXVECTOR3 by( 0.0f, 0.0f, -z );
		viewEye_ += by;
		if ( viewEye_.z > -1.0f )
		{
			viewEye_.z = -1.0f;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void Device::BuildWVPMatrix( const D3DXMATRIX* world, D3DXMATRIX* view, D3DXMATRIX* wvp )
	{
		D3DXMATRIX t,z;
		D3DXMATRIX viewi;

		D3DXMatrixTranslation( &t, viewAt_.x, viewAt_.y, viewAt_.z );
		D3DXMatrixTranslation( &z, viewEye_.x, viewEye_.y, viewEye_.z );

		viewi = z * viewRot_ * t;
		D3DXMatrixInverse( view, NULL, &viewi );

		*wvp = (*world) * (*view) * projection_;
	}

}
