//----------------------------------------------------------------------------
//
//  Title		:	Direct3D.h
//  Desc		:	Helper class to access Direct3D
//----------------------------------------------------------------------------
#ifndef __DIRECT3D_H__
#define __DIRECT3D_H__

#include <d3d9.h>
#include <d3dx9.h>

//#define ENABLE_RENDER_STATS

#ifdef USECD
#undef ENABLE_RENDER_STATS
#endif

#ifdef ENABLE_RENDER_STATS
#define UPDATE_STATS_DRAWPRIM() RenderStats_[ BufferIdx_ ].nTotalDrawPrimCalls_++
#define UPDATE_STATS_DRAWINDEXEDPRIM() RenderStats_[ BufferIdx_ ].nTotalDrawIndexedPrimCalls_++
#define UPDATE_STATS_PRIM( _nPrims ) RenderStats_[ BufferIdx_ ].nTotalPrimsDrawn_ += (_nPrims)
#define UPDATE_STATS_STATE() RenderStats_[ BufferIdx_ ].nStateChanges_++;
#else
#define UPDATE_STATS_DRAWPRIM()
#define UPDATE_STATS_DRAWINDEXEDPRIM()
#define UPDATE_STATS_PRIM( _nPrims )
#define UPDATE_STATS_STATE()
#endif

////////////////////////////////////////////////////////////////////////////////
// Macros

#ifdef _DEBUG
#define ADD_DIRTY_STATE( _x )\
	hcAssert( nDirtyStates_ < D3D_MAX_DIRTYSTATES );\
	DirtyStates_[ nDirtyStates_++ ] = (_x);
#else
#define ADD_DIRTY_STATE( _x ) DirtyStates_[ nDirtyStates_++ ] = (_x);
#endif

#define SAFE_D3D_RELEASE(x) if (x) (x)->Release(); (x) = NULL

#ifdef _DEBUG
#define CHECK_D3DAPI( _fn_ ) hcAssert( !FAILED( (_fn_) ) )
#else
#define CHECK_D3DAPI( _fn_ ) (_fn_)
#endif

#define UPDATE_STATS_STATE()

class HeartEngine;

////////////////////////////////////////////////////////////////////////////////
// Direct3D

class Direct3D
{

	enum d3dVertexShaderVersion
	{
		d3d_VS_NOT_SUPPORTED = 0,
		d3d_VS_1_1,
		d3d_VS_2_0,
		d3d_VS_2_SW,
		d3d_VS_END,
	};

	enum d3dPixelShaderVersion
	{
		d3d_PS_NOT_SUPPORTED = 0,
		d3d_PS_1_1,
		d3d_PS_1_2,
		d3d_PS_1_3,
		d3d_PS_1_4,
		d3d_PS_2_0,
		d3d_PS_2_SW,
		d3d_PS_END,
	};

	enum d3dError
	{
		d3d_ERR_NONE = 0,
		d3d_ERR_OUTOFVIDEOMEMORY,
		d3d_ERR_DRIVER,
		d3d_ERR_UNKNOWN
	};

	enum d3dScreenMode
	{
		d3d_WINDOWED = 0,
		d3d_FULLSCREEN
	};

public:
	
	struct D3DDisplayRes 
	{
		hUint32					Width_;
		hUint32					Height_;
		D3DFORMAT				DepthStencilFormat_;
	};

	struct D3DInitParams
	{
		HWND hWnd;
		D3DDisplayRes pResolution;
		D3DFORMAT Format;
		hBool Windowed; 
		hUint32 AntialiasedQuality_;
		hBool ExactMatch;
		hBool VSync_;
		hFloat minShaderVersion_;
	};

	struct D3DDisplayMode
	{
		unsigned int	Width_;
		unsigned int	Height_;
		D3DFORMAT		RenderFormat_;
		D3DFORMAT		DepthStencilFormat_;
		hBool			DesktopCompatible_;
		unsigned int	Adapter_;
		unsigned int	MultiSampleModes_;
	};

								Direct3D();

	hBool 						createD3D();
	hBool						createDevice( D3DInitParams& params );
	void 						releaseD3D();
	void 						releaseDevice();
	hBool						open( D3DInitParams& params );
	void						buildDeviceList( D3DInitParams& params );
	hBool						chooseDisplayMode( D3DDisplayRes& best, D3DInitParams& params );
	hBool						getBestStencilFormat( D3DFORMAT RenderFormat, D3DFORMAT& pDepthStencilFormat );
	hBool						createAndQueryDevice( D3DPRESENT_PARAMETERS* pD3DPP, DWORD Behaviour );
	hBool						setPresentationParameters( D3DPRESENT_PARAMETERS* pD3DPP, D3DInitParams params );
	void						getShaderSupport();
	int							flipBuffers();
	void						close();

	hBool						reset();

	// Hardware capabilities
// 	hBool						supportsHardwareVertexShaders() const;
// 	hBool						supportsHardwarePixelShaders() const;
 	hFloat						vertexShaderVersion() const;
 	hFloat						pixelShaderVersion() const;
// 	hBool						requiresPow2Textures() const;
// 	hBool						requiresSquareTextures() const;
// 	hBool						hasStencilBuffer() const;
// 	hBool						hasLighting() const;
// 	hBool						hasMultipass() const;
// 	unsigned int				maxSimultaneousTextures() const;
	D3DFORMAT					backBufferFormat() const { return PresentParams_.BackBufferFormat; }
	D3DFORMAT					DepthStencilFormat() const { return PresentParams_.AutoDepthStencilFormat; }
// 	hBool						supportsAnisotropicFiltering() const;
// 	hBool						supportAntiAliasing( unsigned int Level ) const;
	hBool						textureHardwareBug() const;

	//////////////////////////////////////////////////////////////////////////
	// inlines
	inline hBool				isSoftwareDevice() const
	{
		return IsSoftwareDevice_;
	}

	inline LPDIRECT3D9			pD3D() const
	{
		hcAssert( isOpen() );
		return pD3D_;
	}

	inline LPDIRECT3DDEVICE9	pDevice() const
	{
		hcAssert( isOpen() );
		return pDevice_;
	}

	inline hBool				isOpen() const
	{
		return Open_;
	}

	inline const D3DCAPS9&		deviceCaps() const
	{
		hcAssert( isOpen() );
		return DeviceCaps_;
	}
	inline hUint32 				backBufferWidth() const { return Width_; }
	inline hUint32 				backBufferHeight() const { return Height_; }

private:


	LPDIRECT3D9					pD3D_;
	LPDIRECT3DDEVICE9			pDevice_;
	hBool						Open_;
	hUint32						iValidModes_;
	D3DDISPLAYMODE*				pValidDisplayModes_;
	HRESULT						DeviceCreationResult_;
	hBool						IsSoftwareDevice_;
	HWND						hWnd_;
	D3DCAPS9					DeviceCaps_;
	D3DDisplayMode				ChosenDisplayMode_;
	D3DPRESENT_PARAMETERS		PresentParams_;
	hBool						VSync_;
	hUint32						Width_;
	hUint32						Height_;
	d3dScreenMode				Mode_;
	hBool						TextureHardwareBug_;
	hFloat						shaderVerion_;
	hBool						Force_;
	D3DInitParams				Params_;
	D3DDisplayRes				DisplayRes_;

};

// inline hBool Direct3D::supportsHardwareVertexShaders() const
// {
// 	hcAssert( isOpen() );
// 	return SupportsHardwareVertexShader_;
// }
// 
// inline hBool Direct3D::supportsHardwarePixelShaders() const
// {
// 	hcAssert( isOpen() );
// 	return SupportsHardwarePixelShader_;
// }
// 
// inline hBool Direct3D::isSoftwareDevice() const
// {
// 	return IsSoftwareDevice_;
// }
// 
// inline D3DFORMAT Direct3D::backBufferFormat() const
// {
// 	hcAssert( isOpen() );
// 	return ChosenDisplayMode_.RenderFormat_;
// }
inline hBool Direct3D::textureHardwareBug() const
{
	return TextureHardwareBug_;
}

#endif // __DIRECT3D_H__
