/********************************************************************

	filename: 	Direct3D9.h	
	
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

#ifndef DIRECT3D9_H__
#define DIRECT3D9_H__

#include "stdafx.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <map>

namespace Data
{
	class Material;
}

namespace Direct3D
{
	enum ClearFlags
	{
		CLEAR_TARGET,
		CLEAR_Z,
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	extern bool InitD3D();
	extern void FinaliseD3D();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class Device
	{
	public:

		Device() :
			creationFlags_( 0 )
			,hWnd_( NULL )
			,pD3DDevice_( NULL )
		{

		}

		~Device()
		{

		}

		void				DisplayRect();
		const RECT*			pDisplayRect() const { return &displayRect_; }
		bool				CreateResources();
		void				DestroyResources();
		void				BeginRender( u32 width, u32 hieght );
		void				EndRender();
		void				ClearTarget( D3DCOLOR colour, float z );
		void				SetVertexDeclaration( u32 vertexFlags );
		void				SetViewTranslation( const D3DXVECTOR3 t );
		void				TranslateView( const D3DXVECTOR3* by );
		void				ZoomView( float z );
		float				GetViewZoom() const { return viewEye_.z; }
		void				RotateView( float xdegs, float ydegs );
		bool				ApplyMaterial( Data::Material* pmat, const D3DXMATRIX* pworld );
		void				DrawIndexedPrimative( Data::Material* pmat, const D3DXMATRIX* pworld, u32 vertexFlags, u32 indexcound, u32 vertexcount, u32 vertexstride, void* indices, void* vertices );
		void				DrawLightPrimative( const D3DXMATRIX* pworld );
		void				DrawSpherePrimative( const D3DXMATRIX* world, float radius );
		void				DrawConePrimative( const D3DXMATRIX* world, float lenght, float angle );
		void				DrawCameraPrimative( const D3DXMATRIX* pworld );
		void				DrawLocatorPrimative( const D3DXMATRIX* pworld );

		LPDIRECT3DDEVICE9	pD3DDevice_;

	private:

		friend bool CreateD3DDevice( HWND, Device* );
		friend bool DestroyD3DDevice( Device* );

		LPDIRECT3DVERTEXDECLARATION9	CreateVertexDeclaration( u32 vertexFlags );
		void							BuildWVPMatrix( const D3DXMATRIX* world, D3DXMATRIX* view, D3DXMATRIX* wvp );

		struct TextureData
		{
			std::string				file_;
			LPDIRECT3DTEXTURE9		pTex_;
		};

		typedef std::map< u32, TextureData > TextureMap;
		typedef std::map< u32, LPDIRECT3DVERTEXDECLARATION9 > VertDeclMap;

		RECT					displayRect_;
		D3DPRESENT_PARAMETERS	presentParameters_;
		DWORD					creationFlags_;
		HWND					hWnd_;
		D3DCAPS9				caps_;
		VertDeclMap				vertxDecl_;
		LPDIRECT3DTEXTURE9		pProblemTex_;
		LPDIRECT3DTEXTURE9		pCamTex_;
		LPDIRECT3DTEXTURE9		pLightTex_;
		LPD3DXSPRITE			pSpriteRender_;
		LPD3DXMESH				sphereMesh_;
		LPD3DXMESH				coneMesh_;
		TextureMap				loadedTextures_;

		D3DXQUATERNION					viewQuat_;
		D3DXMATRIX						viewRot_;
		D3DXVECTOR3						viewAt_;
		D3DXVECTOR3						viewEye_;

		D3DXMATRIX						view_;
		D3DXMATRIX						projection_;

		
		LPD3DXEFFECT					pRenderEffect_;
		LPDIRECT3DVERTEXDECLARATION9	pVertDecl_;
		D3DXHANDLE						diffuseTextureParam_;
		D3DXHANDLE						worldviewprojParam_;
	};

	extern bool CreateD3DDevice( HWND hWnd, Device* pOutDevice );
	extern bool ResetD3DDevice( Device* pOutDevice );
	extern bool DestroyD3DDevice( Device* pOutDevice );
}

#endif // DIRECT3D9_H__