/********************************************************************
	created:	2009/10/20
	created:	20:10:2009   21:06
	filename: 	RenderState.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HRRENDERSTATE_H__
#define HRRENDERSTATE_H__


#include "hTypes.h"
#include "Common.h"
#include "hRendererConstants.h"
#include "hRenderer.h"
#include "hMaterial.h"

namespace Heart
{

	class hMaterial;
	class hIndexBuffer;
	class hVertexBuffer;
	class VertexDeclaration;
	class hTextureBase;
	class hRenderTargetTexture;
	class DepthSurface;

/*
	Describes the Vertex Input state for the renderer
	Should include the most common changing render state variables
*/
struct hInputStateBlock
{
	hIndexBuffer*					pIndexBuffer_;
	hVertexBuffer*					pVertexBuffer_;
	hVertexDeclaration*				pVertexDecl_;//Vertex Format
	hMatrix							worldMatrix_;
	hMatrix							inverseWorldMatrix_;
	hMatrix							worldInverseTransposeMatirx_;
	hMatrix							viewMatrix_;
	hMatrix							inverseViewMatrix_;
	hMatrix							inverseTransposeViewMatrix_;
	hMatrix							projectionMatrix_;
	hMatrix							inverseProjectionMatrix_;
	hMatrix							viewProjectionMatrix_;
	hMatrix							inverseViewProjectionMatrix_;
	hMatrix							worldViewMatrix_;
	hMatrix							worldViewInverseMatrix_;
	hMatrix							worldInverseTransposeViewMatrix_;
	hMatrix							worldViewInverseTransposeMatrix_;
	hMatrix							worldViewProjectionMatrix_;
	union
	{
		hUint32 AllFlags_;
		struct
		{
			hBool istreamsDirty_ : 1;
			hBool vstreamsDirty_ : 1;
			hBool vfmtDirty_ : 1;
			hBool matrixDirty_ : 1;
		};

	};
};

/*
	This describes each texture state block
*/
struct hTextureStateBlock
{
/*
	D3D10_TEXTURE_ADDRESS_MODE AddressU;
	D3D10_TEXTURE_ADDRESS_MODE AddressV;
	D3D10_TEXTURE_ADDRESS_MODE AddressW;
	FLOAT MipLODBias;
	FLOAT MinLOD;
	FLOAT MaxLOD;
	Texture Object

	All x8
*/
	static const hUint32 MAX_TEXTURES = 8;
	hTextureBase*		pTextures_[ MAX_TEXTURES ];

	union
	{
		hUint32 AllFlags_;
		struct
		{
			hBool t1Dirty_ : 1;
			hBool t2Dirty_ : 1;
			hBool t3Dirty_ : 1;
			hBool t4Dirty_ : 1;
			hBool t5Dirty_ : 1;
			hBool t6Dirty_ : 1;
			hBool t7Dirty_ : 1;
			hBool t8Dirty_ : 1;
		};
	};
};

struct hTargetStateBlock
{
	static const hUint32 MAX_RENDER_TARGETS = 4;


	hRenderTargetTexture*	pTargets_[ MAX_RENDER_TARGETS ];
	hRenderTargetTexture*	pDepthSurface_;
	hViewport				viewport_;
	ScissorRect				scissorRect_;
	union
	{
		hUint32 AllFlags_;
		struct
		{
			hBool rt1Dirty_ : 1;
			hBool rt2Dirty_ : 1;
			hBool rt3Dirty_ : 1;
			hBool rt4Dirty_ : 1;
			hBool viewportDirty_ : 1;
			hBool scissorDirty_ : 1;
			hBool depthDirty_ : 1;
		};
	};
};

/*
	This describes the lighting state
*/
struct hLightingStateBlock
{
//	static const hUint32 MAX_LIGHTS = 4;

// 	hUint32						nLightCount_;
// 	hFloat						type_[ MAX_LIGHTS ];
// 	hFloat						atten_[ MAX_LIGHTS ];
// 	Heart::Vec3			directions_[ MAX_LIGHTS ];
// 	Heart::Vec3			position_[ MAX_LIGHTS ];
// 	Colour						diffuseColours_[ MAX_LIGHTS ];

	hVec4						lightPosition_;
	hVec4						lightDirection_;
	hColour						fogColour_;
	hColour						lightDiffuseColour_;
	hColour						lightSpecularColour_;
	hFloat						fogStart_;
	hFloat						fogRange_;
	hFloat						minRadius_;
	hFloat						maxRadius_;
	hFloat						specExponent_;
	hFloat						lightFalloff_;
	hFloat						lightBrightness_;

	union
	{
		hUint32 AllFlags_;
		struct
		{
			hBool fogStartDirty_ : 1;
			hBool fogRangeDirty_ : 1;
			hBool fogColourDirty_ : 1;
			hBool minRadiusDirty_ : 1;
			hBool maxRadiusDirty_ : 1;
			hBool specExponentDirty_ : 1;
			hBool lightBrightnessDirty_ : 1;
			hBool lightPositionDirty_ : 1;
			hBool lightDirecitonDirty_ : 1;
			hBool lightDiffuseColourDirty_ : 1;
			hBool lightSpecularColourDirty_ : 1;
			hBool lightFalloffDirty_ : 1;
		};
	};
};

class hRenderState
{
public:
	hRenderState( hRenderer& renderer ) 
		: Renderer_( renderer )
		, nDirtyStates_( 0 )
	{
		memset( &inputStateBlock_, hErrorCode, sizeof( hInputStateBlock ) );
		inputStateBlock_.AllFlags_ = 0;
		memset( &texturingStateBlock_, hErrorCode, sizeof( hTextureStateBlock ) );
		texturingStateBlock_.AllFlags_ = 0;
		memset( &texturingStateBlock_, hErrorCode, sizeof( hTextureStateBlock ) );
		texturingStateBlock_.AllFlags_ = 0;
 		memset( &lightingStateBlock_, hErrorCode, sizeof( hLightingStateBlock ) );
 		lightingStateBlock_.AllFlags_ = 0;
	
		pActiveMaterial_ = NULL;
	}
	void					defaultRenderState()
	{
		hMatrix ident;

		ident = hMatrixFunc::identity();


		setIndexStream( NULL );
		setVertexStream( NULL );
		worldMatrix( ident );
		viewMatrix( ident );
		projectionMatrix( ident );
		setMaterial( NULL );
		vertexDecl( NULL );
		setCullMode( RSV_CULL_MODE_NONE );
		setFillMode( RSV_FILL_MODE_SOLID );
		setDepthBias( 0.0f );
		setScissorTest( RSV_DISABLE );
		zTest( RSV_DISABLE );
		zWrite( RSV_DISABLE );
		zCmpFunc( RSV_Z_CMP_LESS );
		stencilTest( RSV_DISABLE );
		blendEnable( RSV_DISABLE );
		blendFunc( RSV_BLEND_FUNC_ADD );
		srcBlend( RSV_BLEND_OP_ONE );
		dstBlend( RSV_BLEND_OP_ZERO );
		stencilTest( RSV_DISABLE );
		stencilFail( RSV_SO_KEEP );
		stencilZFail( RSV_SO_KEEP );
		stencilPass( RSV_SO_KEEP );
		stencilFunc( RSV_SF_CMP_ALWAYS );
		stencilRef( 0 );
		stencilWriteMask( 0xFFFFFFFF );
		stencilMask( 0xFFFFFFFF );
		SetColourWrite1( RSV_COLOUR_WRITE_FULL );
		SetColourWrite2( RSV_COLOUR_WRITE_FULL );
		SetColourWrite3( RSV_COLOUR_WRITE_FULL );
		SetColourWrite4( RSV_COLOUR_WRITE_FULL );
		setTexture( 0, NULL );
		setTexture( 1, NULL );
		setTexture( 2, NULL );
		setTexture( 3, NULL );
		setTexture( 4, NULL );
		setTexture( 5, NULL );
		setTexture( 6, NULL );
		setTexture( 7, NULL );
		hViewport vp;
		vp.x_ = 0;
		vp.y_ = 0;
		vp.width_ = Renderer_.GetWidth();
		vp.height_ = Renderer_.GetHeight();
		SetViewport( vp );
		ScissorRect s;
		s.top_ = 0;
		s.bottom_ = Renderer_.GetHeight();
		s.left_ = 0;
		s.right_ = Renderer_.GetWidth();
		setScissorRect( s );
		SetRenderTargetTexture( 0, NULL );
		SetRenderTargetTexture( 1, NULL );
		SetRenderTargetTexture( 2, NULL );
		SetRenderTargetTexture( 3, NULL );
		SetDepthSurface( NULL );

	}
	// hrInputStateBlock State settings
	void					setIndexStream( hIndexBuffer* pIBuf )
	{
		if ( inputStateBlock_.pIndexBuffer_ != pIBuf )
		{
			inputStateBlock_.pIndexBuffer_ = pIBuf;
			inputStateBlock_.istreamsDirty_ = true;
		}
	}
	void					setVertexStream( hVertexBuffer* pVBuf )
	{
		if ( inputStateBlock_.pVertexBuffer_ != pVBuf )
		{
			inputStateBlock_.pVertexBuffer_ = pVBuf;
			inputStateBlock_.vstreamsDirty_ = true;
		}
	}
	void					worldMatrix( const hMatrix& world )
	{
		inputStateBlock_.worldMatrix_ = world;
		inputStateBlock_.matrixDirty_ = hTrue;
	}
	void					viewMatrix( const hMatrix& view )
	{
		inputStateBlock_.viewMatrix_ = view;
		inputStateBlock_.matrixDirty_ = hTrue;
	}
	void					projectionMatrix( const hMatrix& projection )
	{
		inputStateBlock_.projectionMatrix_ = projection;
		inputStateBlock_.matrixDirty_ = hTrue;
	}
	void					vertexDecl( hVertexDeclaration* val )
	{
		if ( inputStateBlock_.pVertexDecl_ != val )
		{
			inputStateBlock_.pVertexDecl_ = val;
			inputStateBlock_.vfmtDirty_ = hTrue;
		}
	}
	void					setMaterial( hMaterial* pMat )
	{
		if ( pMaterial_ != pMat )
		{
			pMaterial_ = pMat;
		}
	}
	// hrRasterizerStateBlock
	void					setCullMode( RENDER_STATE_VALUE val )
	{
		if ( val != renderStates_[ RS_CULL_MODE ] )
		{
			renderStates_[ RS_CULL_MODE ] = val;
			PushDirtyState( RS_CULL_MODE );
		}
	}
	void					setFillMode( RENDER_STATE_VALUE val )
	{
		if ( val != renderStates_[ RS_FILL_MODE ] )
		{
			renderStates_[ RS_FILL_MODE ] = val;
			PushDirtyState( RS_FILL_MODE );
		}
	}
	void					setDepthBias( hFloat val )
	{
		(void)val;
	}
	void					setScissorTest( RENDER_STATE_VALUE val )
	{
		if ( val != renderStates_[ RS_SCISSOR_TEST ] )
		{
			renderStates_[ RS_SCISSOR_TEST ] = val;
			PushDirtyState( RS_SCISSOR_TEST );
		}
	}
	void					setScissorRect( ScissorRect rect )
	{
		if ( targetStateBlock_.scissorRect_.top_	!= rect.top_	||
			 targetStateBlock_.scissorRect_.bottom_ != rect.bottom_ ||
			 targetStateBlock_.scissorRect_.right_	!= rect.right_	||
			 targetStateBlock_.scissorRect_.left_	!= rect.left_	)
		{
			targetStateBlock_.scissorRect_ = rect;
			targetStateBlock_.scissorDirty_ = hTrue;
		}
	}
	// hrDepthStencilTesting 
	void					zTest( RENDER_STATE_VALUE val )
	{
		if ( val != renderStates_[ RS_Z_TEST ] )
		{
			renderStates_[ RS_Z_TEST ] = val;
			PushDirtyState( RS_Z_TEST );
		}
	}
	void					zWrite( RENDER_STATE_VALUE val )
	{
		if ( val != renderStates_[ RS_Z_WRITE ] )
		{
			renderStates_[ RS_Z_WRITE ] = val;
			PushDirtyState( RS_Z_WRITE );
		}
	}
	void					zCmpFunc( RENDER_STATE_VALUE val )
	{
		if ( val != renderStates_[ RS_Z_COMPARE_FUNCTION ] )
		{
			renderStates_[ RS_Z_COMPARE_FUNCTION ] = val;
			PushDirtyState( RS_Z_COMPARE_FUNCTION );
		}
	}
	void					stencilTest( RENDER_STATE_VALUE val )
	{
		if ( val != renderStates_[ RS_STENCIL_TEST ] )
		{
			renderStates_[ RS_STENCIL_TEST ] = val;
			PushDirtyState( RS_STENCIL_TEST );
		}
	}
	void					stencilFail( RENDER_STATE_VALUE val )
	{
		if ( val != renderStates_[ RS_STENCIL_FAIL ] )
		{
			renderStates_[ RS_STENCIL_FAIL ] = val;
			PushDirtyState( RS_STENCIL_FAIL );
		}
	}
	void					stencilZFail( RENDER_STATE_VALUE val )
	{
		if ( val != renderStates_[ RS_STENCIL_Z_FAIL ] )
		{
			renderStates_[ RS_STENCIL_Z_FAIL ] = val;
			PushDirtyState( RS_STENCIL_Z_FAIL );
		}
	}
	void					stencilPass( RENDER_STATE_VALUE val )
	{
		if ( val != renderStates_[ RS_STENCIL_PASS ] )
		{
			renderStates_[ RS_STENCIL_PASS ] = val;
			PushDirtyState( RS_STENCIL_PASS );
		}
	}
	void					stencilFunc( RENDER_STATE_VALUE val )
	{
		if ( val != renderStates_[ RS_STENCIL_FUNCTION ] )
		{
			renderStates_[ RS_STENCIL_FUNCTION ] = val;
			PushDirtyState( RS_STENCIL_FUNCTION );
		}
	}
	void					stencilRef( hUint32 val )
	{
		if ( val != renderStates_[ RS_STENCIL_REF ] )
		{
			renderStates_[ RS_STENCIL_REF ] = val;
			PushDirtyState( RS_STENCIL_REF );
		}
	}
	void					stencilWriteMask( hUint32 val )
	{
		if ( val != renderStates_[ RS_STENCIL_WRITE_MASK ] )
		{
			renderStates_[ RS_STENCIL_WRITE_MASK ] = val;
			PushDirtyState( RS_STENCIL_WRITE_MASK );
		}
	}
	void					stencilMask( hUint32 val )
	{
		if ( val != renderStates_[ RS_STENCIL_MASK ] )
		{
			renderStates_[ RS_STENCIL_MASK ] = val;
			PushDirtyState( RS_STENCIL_MASK );
		}
	}
	// hrBlendStateBlock
	void					blendEnable( RENDER_STATE_VALUE val )
	{
		if ( val != renderStates_[ RS_ALPHA_BLEND ] )
		{
			renderStates_[ RS_ALPHA_BLEND ] = val;
			PushDirtyState( RS_ALPHA_BLEND );
		}
	}
	void					srcBlend( RENDER_STATE_VALUE val )
	{
		if ( val != renderStates_[ RS_SRC_ALPHA_BLEND ] )
		{
			renderStates_[ RS_SRC_ALPHA_BLEND ] = val;
			PushDirtyState( RS_SRC_ALPHA_BLEND );
		}
	}
	void					dstBlend( RENDER_STATE_VALUE val )
	{
		if ( val != renderStates_[ RS_DST_ALPHA_BLEND ] )
		{
			renderStates_[ RS_DST_ALPHA_BLEND ] = val;
			PushDirtyState( RS_DST_ALPHA_BLEND );
		}
	}
	void					blendFunc( RENDER_STATE_VALUE val )
	{
		if ( val != renderStates_[ RS_ALPHA_BLEND_FUNCTION ] )
		{
			renderStates_[ RS_ALPHA_BLEND_FUNCTION ] = val;
			PushDirtyState( RS_ALPHA_BLEND_FUNCTION );
		}
	}
	void					SetColourWrite1( hUint32 mask )
	{
		if ( mask != renderStates_[RS_COLOUR_WRITE_1] )
		{
			renderStates_[RS_COLOUR_WRITE_1] = mask;
			PushDirtyState( RS_COLOUR_WRITE_1 );
		}
	}
	void					SetColourWrite2( hUint32 mask )
	{
		if ( mask != renderStates_[RS_COLOUR_WRITE_2] )
		{
			renderStates_[RS_COLOUR_WRITE_2] = mask;
			PushDirtyState( RS_COLOUR_WRITE_2 );
		}
	}
	void					SetColourWrite3( hUint32 mask )
	{
		if ( mask != renderStates_[RS_COLOUR_WRITE_2] )
		{
			renderStates_[RS_COLOUR_WRITE_2] = mask;
			PushDirtyState( RS_COLOUR_WRITE_2 );
		}
	}
	void					SetColourWrite4( hUint32 mask )
	{
		if ( mask != renderStates_[RS_COLOUR_WRITE_4] )
		{
			renderStates_[RS_COLOUR_WRITE_4] = mask;
			PushDirtyState( RS_COLOUR_WRITE_4 );
		}
	}
	void*					GetSemanticIDParameterData( hUint32 id );
	// hrTextureStateBlock
	//TODO: remove this so it can be based on semantics
	void					setTexture( hUint32 idx, hTextureBase* val )
	{
		if ( val != texturingStateBlock_.pTextures_[ idx ] )
		{
			texturingStateBlock_.pTextures_[ idx ] = val;
			texturingStateBlock_.AllFlags_ |= 0x00000001 << idx;
		}
	}
	void					SetRenderTargetTexture( hUint32 idx, hRenderTargetTexture* pTarget )
	{
		if ( idx < 4 )
		{
			if( pTarget != targetStateBlock_.pTargets_[ idx ] )
			{
				targetStateBlock_.pTargets_[ idx ] = pTarget;
				targetStateBlock_.AllFlags_ |= 0x00000001 << idx;
			}
		}
	}
	void					SetDepthSurface( hRenderTargetTexture* pSurface )
	{
		if ( targetStateBlock_.pDepthSurface_ != pSurface )
		{
			targetStateBlock_.pDepthSurface_ = pSurface;
			targetStateBlock_.depthDirty_ = hTrue;
		}
	}
	void					SetViewport( const hViewport& viewport )
	{
		targetStateBlock_.viewport_ = viewport;
		targetStateBlock_.viewportDirty_ = hTrue;
	}
	void					SetFogStart( const hFloat start )
	{
		if ( lightingStateBlock_.fogStart_ != start )
		{
			lightingStateBlock_.fogStart_ = start;
			lightingStateBlock_.fogStartDirty_ = hTrue;
		}
	}
	void					SetFogRange( const hFloat range )
	{
		if ( lightingStateBlock_.fogRange_ != range )
		{
			lightingStateBlock_.fogRange_ = range;
			lightingStateBlock_.fogRangeDirty_ = hTrue;
		}
	}
	void					SetFogColour( const hColour& colour )
	{
		if ( lightingStateBlock_.fogColour_ != colour )
		{
			lightingStateBlock_.fogColour_ = colour;
			lightingStateBlock_.fogColourDirty_ = hTrue;
		}
	}
	void					SetLightBrightness( hFloat val )
	{
		if ( lightingStateBlock_.lightBrightness_ != val )
		{
			lightingStateBlock_.lightBrightness_ = val;
			lightingStateBlock_.lightBrightnessDirty_ = hTrue;
		}
	}
	void					SetLightDiffuse( const hColour& colour )
	{
		if ( lightingStateBlock_.lightDiffuseColour_ != colour )
		{
			lightingStateBlock_.lightDiffuseColour_ = colour;
			lightingStateBlock_.lightDiffuseColourDirty_ = hTrue;
		}
	}
	void					SetLightSpecular( const hColour& colour )
	{
		if ( lightingStateBlock_.lightSpecularColour_ != colour )
		{
			lightingStateBlock_.lightSpecularColour_ = colour;
			lightingStateBlock_.lightSpecularColourDirty_ = hTrue;
		}
	}
	void					SetLightPosition( const hVec3& pos )
	{
		if ( lightingStateBlock_.lightPosition_ != hVec4( pos ) )
		{
			lightingStateBlock_.lightPosition_ = hVec4( pos );
			lightingStateBlock_.lightPositionDirty_ = hTrue;
		}
	}
	void					SetLightDirection( const hVec3& dir )
	{
		if ( lightingStateBlock_.lightDirection_ != hVec4( dir ) )
		{
			lightingStateBlock_.lightDirection_ = hVec4( dir );
			lightingStateBlock_.lightDirecitonDirty_ = hTrue;
		}
	}
	void					SetLightRadius( const hFloat minr, const hFloat maxr )
	{
		if ( lightingStateBlock_.minRadius_ != minr || lightingStateBlock_.maxRadius_ != maxr )
		{
			lightingStateBlock_.minRadius_ = minr;
			lightingStateBlock_.maxRadius_ = maxr;
			lightingStateBlock_.minRadiusDirty_ = hTrue;
			lightingStateBlock_.maxRadiusDirty_ = hTrue;
		}
	}
	void					SetSpecularExponent( const hFloat exp )
	{
		if ( lightingStateBlock_.specExponent_ != exp )
		{
			lightingStateBlock_.specExponent_ = exp;
			lightingStateBlock_.specExponentDirty_ = hTrue;
		}
	}
	void					SetLightFalloff( const hFloat fo )
	{
		if ( lightingStateBlock_.lightFalloff_ != fo )
		{
			lightingStateBlock_.lightFalloff_ = fo;
			lightingStateBlock_.lightFalloffDirty_ = hTrue;
		}
	}

	void					PushDirtyState( RENDER_STATE dirty )
	{
		if ( !isDirty_[ dirty ] )
		{
			dirtyStates_[ nDirtyStates_++ ] = (hUint32)dirty;
			isDirty_[ dirty ] = hTrue;
		}
	}

	void					ClearDirtyStates()
	{
		inputStateBlock_.AllFlags_ = 0;
		texturingStateBlock_.AllFlags_ = 0;
		targetStateBlock_.AllFlags_ = 0;
		lightingStateBlock_.AllFlags_ = 0;

		memset( &isDirty_[ 0 ], 0, RS_MAX*sizeof(hBool) );
		nDirtyStates_ = 0;
	}

	//	commit
	void					commit();

	void					EndFrame()
	{
		if ( pActiveMaterial_ )
		{
			//pActiveMaterial_->EndEffect();
			pActiveMaterial_ = NULL;
		}

		//need to invalidate texture states, etc...?
		for ( hUint32 i = 0; i < hTextureStateBlock::MAX_TEXTURES; ++i )
		{
			texturingStateBlock_.pTextures_[ i ] = NULL;
		}
	}

	void					commitStateAndCopyTo( hRenderState& currentState )
	{
		//TODO:
		hcBreak;
	}

private:

	friend class hRenderer;

	hRenderer&														Renderer_;
	hInputStateBlock												inputStateBlock_;
	hTargetStateBlock												targetStateBlock_;
	hLightingStateBlock												lightingStateBlock_;
	hArray< hUint32/*RENDER_STATE_VALUE*/, RS_MAX >		renderStates_;
	hArray< hBool, RS_MAX >								isDirty_;
	hArray< hUint32, RS_MAX >								dirtyStates_;
	hUint32															nDirtyStates_;
	hTextureStateBlock												texturingStateBlock_;
	hMaterial*														pMaterial_;
	hMaterial*														pActiveMaterial_;
	hFloat															screenParameters_[4];
	//hrLightingStateBlock											lightingStateBlock_;
};


}

#endif // HRRENDERSTATE_H__
