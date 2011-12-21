//----------------------------------------------------------------------------
//
//  Title		:	Direct3D.h
//  Desc		:	Helper class to access Direct3D
//----------------------------------------------------------------------------

#include <d3dx9.h>
#include "hArray.h"
#include "Direct3D.h"
#include <algorithm>
#include "hDebugMacros.h"

//#define DUMP_FLUSH_STATES
#define DEBUG_FLUSH			hcPrintf

////////////////////////////////////////////////////////////////////////////////
// Constants

const float DEFAULT_GAMMA_VALUE = 130.0f;

////////////////////////////////////////////////////////////////////////////////
// Direct3D

Direct3D::Direct3D()
:	Open_( hFalse ),
	pD3D_( NULL ),
	pDevice_( NULL ),
	hWnd_( NULL ),
	DeviceCreationResult_( D3D_OK ),
	Force_( hFalse ),
	IsSoftwareDevice_( hFalse ),
	VSync_( 0 ),
	TextureHardwareBug_( hFalse )
{
}

#if 0
////////////////////////////////////////////////////////////////////////////////
// determineShaderSupport
void Direct3D::determineShaderSupport()
{
	// Check for vertex shader support
	unsigned int MajorVersion = ( DeviceCaps_.VertexShaderVersion & 0xff00 ) >> 8;
	unsigned int MinorVersion = ( DeviceCaps_.VertexShaderVersion & 0x00ff );

	if ( ( MajorVersion == 0 ) || ( isSoftwareDevice() ) )
	{
		SupportsHardwareVertexShader_ = false;
	}
	else if ( MajorVersion >= 1 )
	{
		SupportsHardwareVertexShader_ = true;
	}

	VertexShaderVersion_ = d3d_VS_1_1;

	// Check pixel shader support
	MajorVersion = ( DeviceCaps_.PixelShaderVersion & 0xff00 ) >> 8;
	MinorVersion = ( DeviceCaps_.PixelShaderVersion & 0x00ff );

	if ( MajorVersion == 0 )
	{
		SupportsHardwarePixelShader_ = false;
		PixelShaderVersion_ = d3d_PS_2_SW;
	}
	else if ( MajorVersion == 1 )
	{
		SupportsHardwarePixelShader_ = true;
		switch ( MinorVersion )
		{
		case 1:
		default:
			PixelShaderVersion_ = d3d_PS_1_1;
			break;
		case 2:
			PixelShaderVersion_ = d3d_PS_1_2;
			break;
		case 3:
			PixelShaderVersion_ = d3d_PS_1_3;
			break;
		case 4:
			PixelShaderVersion_ = d3d_PS_1_4;
			break;
		}
	}
	else if ( MajorVersion >= 2 )
	{
		SupportsHardwarePixelShader_ = true;
		PixelShaderVersion_ = d3d_PS_2_0;
	}
}
#endif

////////////////////////////////////////////////////////////////////////////////
// buildDeviceList
void Direct3D::buildDeviceList( D3DInitParams& params )
{
	hcAssert( pD3D_ != NULL );

	hUint32 AdaptorCount = pD3D_->GetAdapterCount();

	// We should Get all the adapters in the system (usually only one),
	// but I won't bother
	UINT CurAdapterIdent = D3DADAPTER_DEFAULT;
	D3DDISPLAYMODE DesktopDisplay;

	pD3D_->GetAdapterDisplayMode( CurAdapterIdent, &DesktopDisplay );

	// in windowed mode, we must match the current display format
	if ( params.Windowed == hTrue )
	{
		params.Format = DesktopDisplay.Format;
		params.AntialiasedQuality_ = 0;
	}
	else if ( params.pResolution.Width_ == 0 || params.pResolution.Height_ == 0 )
	{
		params.pResolution.Width_ = DesktopDisplay.Width;
		params.pResolution.Height_ = DesktopDisplay.Height;
	}

	// enumerate the possible display modes
	// store the built lists for the next stage
	hUint32 nModes = pD3D_->GetAdapterModeCount( CurAdapterIdent, params.Format );
	iValidModes_ = 0;
	pValidDisplayModes_ = new( hRendererHeap ) D3DDISPLAYMODE[ nModes ];

	for ( hUint32 iMode = 0; iMode < nModes; ++iMode )
	{
		D3DDISPLAYMODE DisplayMode;
		D3DCAPS9 Caps;
		pD3D_->EnumAdapterModes( CurAdapterIdent, params.Format, iMode, &DisplayMode );

		//check the caps of the format
		if ( SUCCEEDED( pD3D_->GetDeviceCaps( CurAdapterIdent, D3DDEVTYPE_HAL, &Caps ) ) )
		{
			// check the format can be used as a render target
			if ( FAILED( pD3D_->CheckDeviceType( CurAdapterIdent, D3DDEVTYPE_HAL, DesktopDisplay.Format, params.Format, FALSE ) ) )
			{
				continue;
			}
			// check that the format can support the required antialised quality
			if ( params.AntialiasedQuality_ == 1 )
			{
				if ( FAILED( pD3D_->CheckDeviceMultiSampleType( CurAdapterIdent,
					D3DDEVTYPE_HAL,
					params.Format,
					FALSE, 
					D3DMULTISAMPLE_2_SAMPLES, 
					NULL ) ) )
				{
					continue;
				}
			}
			else if ( params.AntialiasedQuality_ == 2 )
			{
				if ( SUCCEEDED( pD3D_->CheckDeviceMultiSampleType( CurAdapterIdent,
					D3DDEVTYPE_HAL,
					params.Format,
					FALSE, 
					D3DMULTISAMPLE_4_SAMPLES,
					NULL ) ) )
				{
					continue;
				}
			}

			// TODO: select the best stencil format
			// TODO: other sanity checks on the format
			// add the format to the valid formats list
			pValidDisplayModes_[ iValidModes_++ ] = DisplayMode;
		}
	}

	// error if no formats are found
	if ( iValidModes_ == 0 )
	{
		delete[] pValidDisplayModes_;
		pValidDisplayModes_ = NULL;
	}

}

////////////////////////////////////////////////////////////////////////////////
// chooseDisplayMode

#define DIFF( a, b ) ( ( b.Width - a.Width_ )*( b.Width - a.Width_ ) + ( b.Height - a.Height_ )*( b.Height - a.Height_ ) )

hBool Direct3D::chooseDisplayMode( D3DDisplayRes& best, D3DInitParams& params )
{
	hcAssert( iValidModes_ > 0 );

	if ( iValidModes_ == 0 )
	{
		return hFalse;
	}

	hUint32 smalldist = DIFF( params.pResolution, pValidDisplayModes_[ 0 ] );
	best.Width_ = pValidDisplayModes_[ 0 ].Width;
	best.Height_ = pValidDisplayModes_[ 0 ].Height;

	for ( hUint32 iMode = 0; iMode < iValidModes_; ++iMode )
	{
		if ( DIFF( params.pResolution, pValidDisplayModes_[ iMode ] ) < smalldist )
		{
			best.Width_ = pValidDisplayModes_[ iMode ].Width;
			best.Height_ = pValidDisplayModes_[ iMode ].Height;
			smalldist = DIFF( params.pResolution, pValidDisplayModes_[ iMode ] );
		}
	}

	// choose best stencil format
	if ( !getBestStencilFormat( params.Format, best.DepthStencilFormat_ ) )
	{
		return hFalse;
	}

	return hTrue;
}

#define CHECK_DEPTHSTENCIL_FORMAT( _depthstencil )\
if ( SUCCEEDED( pD3D_->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, RenderFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, (_depthstencil) ) ) )\
{\
	if ( SUCCEEDED( pD3D_->CheckDepthStencilMatch( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, RenderFormat, RenderFormat, (_depthstencil) ) ) )\
	{\
		pDepthStencilFormat = _depthstencil;\
		return hTrue;\
	}\
}

hBool Direct3D::getBestStencilFormat( D3DFORMAT RenderFormat, D3DFORMAT& pDepthStencilFormat )
{
	// Prefer a depth stencil format with a stencil buffer
	// otherwise choose the format with the most depth bits
	CHECK_DEPTHSTENCIL_FORMAT( D3DFMT_D24S8 );
	//CHECK_DEPTHSTENCIL_FORMAT( D3DFMT_D24X4S4 );
	//CHECK_DEPTHSTENCIL_FORMAT( D3DFMT_D15S1 );
	//CHECK_DEPTHSTENCIL_FORMAT( D3DFMT_D32 );
	//CHECK_DEPTHSTENCIL_FORMAT( D3DFMT_D24X8 );
	//CHECK_DEPTHSTENCIL_FORMAT( D3DFMT_D16 );
	return hFalse;
}

////////////////////////////////////////////////////////////////////////////////
// createAndQueryDevice
//#define NVHUD
hBool Direct3D::createAndQueryDevice( D3DPRESENT_PARAMETERS* pD3DPP, DWORD Behaviour )
{
	// Attempt to create a D3D device on the default adapter
	// with the requested behavior

	pD3DPP->BackBufferCount = 1;

	DeviceCreationResult_ = pD3D_->CreateDevice( D3DADAPTER_DEFAULT,
												 D3DDEVTYPE_HAL,
												 hWnd_,
												 //TODO: FIX! Currently, things are created across
												 // multiple threads, this is bad and needs fixing ASAP!
												 Behaviour /*| D3DCREATE_MULTITHREADED*/,
												 pD3DPP,
												 &pDevice_ );

	if ( FAILED( DeviceCreationResult_ ) )
	{
		pDevice_ = NULL;
		return hFalse;
	}

	pDevice_->GetDeviceCaps( &DeviceCaps_ );

	if ( Behaviour & D3DCREATE_SOFTWARE_VERTEXPROCESSING )
	{
		IsSoftwareDevice_ = hTrue;
	}
	else
	{
		IsSoftwareDevice_ = hFalse;
	}

	// Check for stencil buffer support
// 	DWORD StencilCaps = DeviceCaps_.StencilCaps;
// 
// 	if ( D3DHIsStencilFormat( ChosenDisplayMode_.DepthStencilFormat_ ) &&
// 		( ( StencilCaps & D3DSTENCILCAPS_DECR ) != 0 ) &&
// 		( ( StencilCaps & D3DSTENCILCAPS_INCR ) != 0 ) &&
// 		( ( StencilCaps & D3DSTENCILCAPS_REPLACE ) != 0 ) &&
// 		( ( StencilCaps & D3DSTENCILCAPS_KEEP ) != 0 ) )
// 	{
// 		HasStencilBuffer_ = hTrue;
// 	}
// 	else
// 	{
// 		HasStencilBuffer_ = hFalse;
// 	}

	// Check for hardware shader support
// 	determineShaderSupport();
// 
// 	if( !SupportsHardwarePixelShader_ && ( Behaviour == D3DCREATE_HARDWARE_VERTEXPROCESSING ) )
// 	{
// 		SAFE_D3D_RELEASE( pDevice_ );
// 		return hFalse;
// 	}

	return hTrue;
}

////////////////////////////////////////////////////////////////////////////////
// createD3D

hBool Direct3D::createD3D()
{
	pD3D_ = Direct3DCreate9( D3D_SDK_VERSION );
	hcAssert( pD3D_ != NULL );
	hcPrintf( "Direct3D::createD3D: Created Direct3D9 object\n" );
	return ( pD3D_ != NULL );
}

////////////////////////////////////////////////////////////////////////////////
// setPresentationParameters

hBool Direct3D::setPresentationParameters( D3DPRESENT_PARAMETERS* pD3DPP, D3DInitParams params )
{
	hcAssert( pD3DPP != NULL );
	hcAssert( pD3D_ != NULL );
	hcAssert( hWnd_ != NULL );

	// Choose the best enumerated display mode from the user requirements
	D3DDisplayRes Res;

	if ( chooseDisplayMode( Res, params ) )
	{
		ZeroMemory( pD3DPP, sizeof( D3DPRESENT_PARAMETERS ) );

		pD3DPP->Windowed				= ( Mode_ == d3d_WINDOWED );
		pD3DPP->hDeviceWindow			= hWnd_;
		pD3DPP->SwapEffect				= D3DSWAPEFFECT_DISCARD;
		pD3DPP->BackBufferFormat		= params.Format;
		pD3DPP->BackBufferWidth			= Res.Width_;
		pD3DPP->BackBufferHeight		= Res.Height_;
		pD3DPP->EnableAutoDepthStencil	= hFalse;//hTrue;
		pD3DPP->AutoDepthStencilFormat	= Res.DepthStencilFormat_;
		pD3DPP->BackBufferCount			= 2;

		if ( Mode_ == d3d_WINDOWED )
		{
			pD3DPP->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
			pD3DPP->MultiSampleType	= D3DMULTISAMPLE_NONE;
		}
		else
		{
			if ( params.AntialiasedQuality_ == 2 )
			{
				pD3DPP->MultiSampleType	= D3DMULTISAMPLE_4_SAMPLES;
			}
			else if ( params.AntialiasedQuality_ == 1 )
			{
				pD3DPP->MultiSampleQuality = D3DMULTISAMPLE_2_SAMPLES;
			}
			else
			{
				pD3DPP->MultiSampleType = D3DMULTISAMPLE_NONE;
			}
		}

		if( VSync_ )
		{
			pD3DPP->PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		}
		else
		{
			pD3DPP->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		}

		DisplayRes_ = Res;

		delete[] pValidDisplayModes_;

		return hTrue;
	}
	else
	{
		return hFalse;
	}
}

////////////////////////////////////////////////////////////////////////////////
// createDevice

hBool Direct3D::createDevice( D3DInitParams& params )
{
	hcAssert( pD3D_ != NULL );

	pDevice_ = NULL;

	// Sort formats by the closest match to request

	if ( setPresentationParameters( &PresentParams_, params ) )
	{

		// Use software processing for debugging
		// createAndQueryDevice( &PresentParams_, D3DCREATE_SOFTWARE_VERTEXPROCESSING );

		// Attempt to create a hardware vertex processing device, if failure
		// use a software device
		if ( !createAndQueryDevice( &PresentParams_, D3DCREATE_HARDWARE_VERTEXPROCESSING ) )
		{
			if ( !createAndQueryDevice( &PresentParams_, D3DCREATE_MIXED_VERTEXPROCESSING ) )
			{
				createAndQueryDevice( &PresentParams_, D3DCREATE_SOFTWARE_VERTEXPROCESSING );
			}
		}

		hcAssert( pDevice_ != NULL );
	}

	pD3D_->GetDeviceCaps( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &DeviceCaps_ );

	// Check that the device has a DirectX9 driver
	if ( pDevice_ != NULL )
	{
		if ( ( DeviceCaps_.MaxStreams == 0 ) || ( ( DeviceCaps_.DevCaps2 & D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES ) == 0 ) )
		{
			// Not a DirectX9 driver
			releaseDevice();

		}
	}

	return ( pDevice_ != NULL );
}

////////////////////////////////////////////////////////////////////////////////
// releaseDevice

void Direct3D::releaseDevice()
{
	SAFE_D3D_RELEASE( pDevice_ );
}

////////////////////////////////////////////////////////////////////////////////
// releaseD3D

void Direct3D::releaseD3D()
{
	SAFE_D3D_RELEASE( pD3D_ );
}

////////////////////////////////////////////////////////////////////////////////
// open

hBool Direct3D::open( D3DInitParams& params )
{
	Direct3D::d3dScreenMode ScreenMode;

	if( params.Windowed == hTrue )
	{
		ScreenMode = d3d_WINDOWED;
	}
	else
	{
		ScreenMode = d3d_FULLSCREEN;
	}

	hWnd_ = params.hWnd;
	Mode_ = ScreenMode;
	VSync_ = params.VSync_;

	// Create the Direct3D object which is needed to create the device
	if ( !createD3D() )
	{
		return hFalse;
	}

	// build a list of valid display formats
	buildDeviceList( params );

	// Create the device
	if ( !createDevice( params ) )
	{
		//hcPrintf( "Direct3D::open: Device creation failed\n" );
		MessageBox( NULL, "Direct3D::open: Device creation failed.\nWill now Exit.", NULL, MB_OK );
		exit( 1 );
	}

	//calculate pixel & vertex shader support
	getShaderSupport();

	if ( shaderVerion_ < params.minShaderVersion_ )
	{
		hChar str[32];
		sprintf( str, "Support for shader model %.1f is required.\nWill now exit.", params.minShaderVersion_ );
		MessageBox( NULL, str, NULL, MB_OK );
		exit( 1 );
	}

	Params_ = params;
	Width_ = DisplayRes_.Width_;
	Height_ = DisplayRes_.Height_;

	Open_ = hTrue;

	return hTrue;
}

////////////////////////////////////////////////////////////////////////////////
// close

void Direct3D::close()
{
	releaseDevice();
	releaseD3D();
	Open_ = hFalse;
}

////////////////////////////////////////////////////////////////////////////////
// reset

hBool Direct3D::reset()
{
	HRESULT hr = pDevice_->TestCooperativeLevel();

	if( hr != S_OK )
	{
		if( hr == D3DERR_DEVICELOST )
			hcPrintf("D3D RESET : Device Lost\n");
		if( hr == D3DERR_DEVICENOTRESET )
			hcPrintf("D3D RESET : Not Reset\n");

		setPresentationParameters( &PresentParams_, Params_ );

		hr = pDevice_->Reset( &PresentParams_ );

		return hTrue;
	}

	return hFalse;
}

////////////////////////////////////////////////////////////////////////////////
// flipBuffers

int Direct3D::flipBuffers()
{
	hcAssert( isOpen() );

	RECT SourceRect;
	SourceRect.bottom = Height_;
	SourceRect.top = 0;
	SourceRect.right = Width_;
	SourceRect.left= 0;

	RECT DestRect;
	DestRect.bottom = Height_;
	DestRect.top = 0;
	DestRect.right = Width_;
	DestRect.left= 0;

	HRESULT hr;
	
	if( Mode_ == d3d_WINDOWED )
	{
		hr = pDevice_->Present( &SourceRect, &DestRect, NULL, NULL );
	}
	else
	{
		hr = pDevice_->Present( NULL, NULL, NULL, NULL );
	}


	if ( FAILED( hr ) )
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

void Direct3D::getShaderSupport()
{
	WORD Minor = ( DeviceCaps_.PixelShaderVersion & 0xFFFF0000 ) >> 16;
	WORD Major = ( DeviceCaps_.PixelShaderVersion & 0x0000FFFF ) >> 8;

	shaderVerion_ = (hFloat)Major;
}