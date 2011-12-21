/********************************************************************

	filename: 	hrRenderState.cpp	
	
	Copyright (c) 12:3:2011 James Moran
	
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

#include "Common.h"
#include "hRenderState.h"
#include "hTexture.h"

namespace Heart
{


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void* hRenderState::GetSemanticIDParameterData( EffectSemantics::EffectSemanticID id )
	{
		switch ( id ) 
		{
		case EffectSemantics::EFFECTSEMANTICID_SCREENPARAMETERS:
			return screenParameters_;
		case EffectSemantics::EFFECTSEMANTICID_WORLDMATRIX:
			return &inputStateBlock_.worldMatrix_;
		case EffectSemantics::EFFECTSEMANTICID_INVERSEWORLDMATRIX:
			return &inputStateBlock_.inverseWorldMatrix_;
		case EffectSemantics::EFFECTSEMANTICID_WORLDINVERSETRANSPOSEMATRIX:
			return &inputStateBlock_.worldInverseTransposeMatirx_;
		case EffectSemantics::EFFECTSEMANTICID_VIEWMATRIX:
			return &inputStateBlock_.viewMatrix_;
		case EffectSemantics::EFFECTSEMANTICID_INVERSEVIEWMATRIX:
			return &inputStateBlock_.inverseViewMatrix_;
		case EffectSemantics::EFFECTSEMANTICID_INVERSETRANSPOSEVIEWMATRIX:
			return &inputStateBlock_.inverseTransposeViewMatrix_;
		case EffectSemantics::EFFECTSEMANTICID_PROJECTIONMATRIX:
			return &inputStateBlock_.projectionMatrix_;
		case EffectSemantics::EFFECTSEMANTICID_INVERSEPROJECTIONMATRIX:
			return &inputStateBlock_.inverseProjectionMatrix_;
		case EffectSemantics::EFFECTSEMANTICID_VIEWPROJECTIONMATRIX:
			return &inputStateBlock_.viewProjectionMatrix_;
		case EffectSemantics::EFFECTSEMANTICID_INVERSEVIEWPROJECTIONMATRIX:
			return &inputStateBlock_.inverseViewProjectionMatrix_;
		case EffectSemantics::EFFECTSEMANTICID_WORLDVIEWMATRIX:
			return &inputStateBlock_.worldViewMatrix_;
		case EffectSemantics::EFFECTSEMANTICID_WORLDVIEWINVERSEMATRIX:
			return &inputStateBlock_.worldViewInverseMatrix_;
		case EffectSemantics::EFFECTSEMANTICID_WORLDINVERSETRANSPOSEVIEWMATRIX:
			return &inputStateBlock_.worldInverseTransposeViewMatrix_;
		case EffectSemantics::EFFECTSEMANTICID_WORLDVIEWINVERSETRANSPOSEMATRIX:
			return &inputStateBlock_.worldViewInverseTransposeMatrix_;
		case EffectSemantics::EFFECTSEMANTICID_WORLDVIEWPROJECTIONMATRIX:
			return &inputStateBlock_.worldViewProjectionMatrix_;
		case EffectSemantics::EFFECTSEMANTICID_MATERIALAMBIENT:
			return NULL;
		case EffectSemantics::EFFECTSEMANTICID_MATERIALDIFFUSE:
			return NULL;
		case EffectSemantics::EFFECTSEMANTICID_MATERIALEMISSIVE:
			return NULL;
		case EffectSemantics::EFFECTSEMANTICID_MATERIALSPECULAR:
			return NULL;
		case EffectSemantics::EFFECTSEMANTICID_MATERIALSPECPOWER:
			return NULL;
		case EffectSemantics::EFFECTSEMANTICID_FOGSTART:
			return &lightingStateBlock_.fogStart_;
		case EffectSemantics::EFFECTSEMANTICID_FOGRANGE:
			return &lightingStateBlock_.fogRange_;
		case EffectSemantics::EFFECTSEMANTICID_FOGCOLOUR:
			return &lightingStateBlock_.fogColour_;
		case EffectSemantics::EFFECTSEMANTICID_LIGHTBRIGHTNESS:
			return &lightingStateBlock_.lightBrightness_;
		case EffectSemantics::EFFECTSEMANTICID_LIGHTPOSITION:
			return &lightingStateBlock_.lightPosition_;
		case EffectSemantics::EFFECTSEMANTICID_LIGHTDIRECTION:
			return &lightingStateBlock_.lightDirection_;
		case EffectSemantics::EFFECTSEMANTICID_LIGHTDIFFUSE:
			return &lightingStateBlock_.lightDiffuseColour_;
		case EffectSemantics::EFFECTSEMANTICID_LIGHTSPECULAR:
			return &lightingStateBlock_.lightSpecularColour_;
		case EffectSemantics::EFFECTSEMANTICID_LIGHTMINRADIUS:
			return &lightingStateBlock_.minRadius_;
		case EffectSemantics::EFFECTSEMANTICID_LIGHTMAXRADIUS:
			return &lightingStateBlock_.maxRadius_;
		case EffectSemantics::EFFECTSEMANTICID_LIGHTEXPONENT:
			return &lightingStateBlock_.specExponent_;
		case EffectSemantics::EFFECTSEMANTICID_LIGHTFALLOFF:
			return &lightingStateBlock_.lightFalloff_;
		case EffectSemantics::EFFECTSEMANTICID_DIFFUSETEXTURE0:
			return NULL;
		case EffectSemantics::EFFECTSEMANTICID_DIFFUSETEXTURE1:
			return NULL;
		case EffectSemantics::EFFECTSEMANTICID_DIFFUSETEXTURE2:
			return NULL;
		case EffectSemantics::EFFECTSEMANTICID_DIFFUSETEXTURE3:
			return NULL;
		case EffectSemantics::EFFECTSEMANTICID_NORMALTEXTURE:
			return NULL;
		case EffectSemantics::EFFECTSEMANTICID_SPECULARTEXTURE:
			return NULL;
		case EffectSemantics::EFFECTSEMANTICID_LIGHTMAPTEXTURE:
			return NULL;
		case EffectSemantics::EFFECTSEMANTICID_SHADOWMAPTEXTURE:
			return NULL;
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderState::commit()
	{
        /*
		//rebuild screen parameters
		screenParameters_[0] = (hFloat)Renderer_.Width();
		screenParameters_[1] = (hFloat)Renderer_.Height();
		screenParameters_[2] = 1.0f/(hFloat)Renderer_.Width();
		screenParameters_[3] = 1.0f/(hFloat)Renderer_.Height();

		if ( pMaterial_ != pActiveMaterial_  )
		{
			if ( pActiveMaterial_ )
			{
				pActiveMaterial_->EndEffect();
			}

			pActiveMaterial_ = pMaterial_;

			if ( pActiveMaterial_ )
			{
				pActiveMaterial_->ApplyEffect();

				//set textures
				hUint32 paramCount = pActiveMaterial_->GetShaderParameterCount();
				for ( hUint32 i = 0; i < paramCount; ++i )
				{
					const hShaderParameter* pParam = pActiveMaterial_->GetShaderParameter( i );
					if ( pParam->type_ == MATERIALCONSTTYPE_TEXTURE && pParam->boundTexture_.HasData() )
					{
						pActiveMaterial_->pImpl()->SetTextureParameter( i, pParam->boundTexture_->pImpl() );
					}
				}
			}

			//force all textures to be re-sent
			inputStateBlock_.matrixDirty_ = hTrue;
			texturingStateBlock_.AllFlags_ = ~0U;
			lightingStateBlock_.AllFlags_  = ~0U;
		}
		// Render Target Commit
		if ( targetStateBlock_.AllFlags_ )
		{
			hUint32 tarbit = 1;
			for ( hUint32 i = 0; i < hTargetStateBlock::MAX_RENDER_TARGETS; ++i )
			{
				if ( targetStateBlock_.pTargets_[ i ] && (targetStateBlock_.AllFlags_ & tarbit) == tarbit )
				{
					Renderer_.SetRenderTarget( targetStateBlock_.pTargets_[ i ], i );
				}
				tarbit <<= 1;
			}

			if ( targetStateBlock_.depthDirty_ )
			{
				Renderer_.SetDepthSurface( targetStateBlock_.pDepthSurface_ );
			}

			if ( targetStateBlock_.viewportDirty_ )
			{
				Renderer_.SetViewport( targetStateBlock_.viewport_ );
			}

			if ( targetStateBlock_.scissorDirty_ )
			{
				Renderer_.SetScissorRect( targetStateBlock_.scissorRect_ );
			}
		}

		//input State Block commit
		if ( inputStateBlock_.AllFlags_ )
		{
			if ( inputStateBlock_.istreamsDirty_ && inputStateBlock_.pIndexBuffer_ )
			{
				Renderer_.SetIndexBuffer( inputStateBlock_.pIndexBuffer_ );
			}

			if ( inputStateBlock_.vstreamsDirty_ && inputStateBlock_.pVertexBuffer_ )
			{
				Renderer_.SetVertexBuffer( inputStateBlock_.pVertexBuffer_ );
			}

			if ( inputStateBlock_.vfmtDirty_ && inputStateBlock_.pVertexDecl_ )
			{
				Renderer_.SetVertexFormat( inputStateBlock_.pVertexDecl_ );
			}
		}

		if ( pActiveMaterial_ )
		{
			//build worldView & worldViewProjection matrices 
			if ( inputStateBlock_.matrixDirty_ )
			{
				hMatrix::inverse( &inputStateBlock_.worldMatrix_, &inputStateBlock_.inverseWorldMatrix_ );
				hMatrix::transpose( &inputStateBlock_.inverseWorldMatrix_, &inputStateBlock_.worldInverseTransposeMatirx_ );
				hMatrix::inverse( &inputStateBlock_.viewMatrix_, &inputStateBlock_.inverseViewMatrix_ );
				hMatrix::transpose( &inputStateBlock_.inverseViewMatrix_, &inputStateBlock_.inverseTransposeViewMatrix_ );
				hMatrix::mult( &inputStateBlock_.worldInverseTransposeMatirx_, &inputStateBlock_.viewMatrix_, &inputStateBlock_.worldInverseTransposeViewMatrix_ );
				hMatrix::mult( &inputStateBlock_.worldMatrix_, &inputStateBlock_.viewMatrix_, &inputStateBlock_.worldViewMatrix_ );
				hMatrix::mult( &inputStateBlock_.worldViewMatrix_, &inputStateBlock_.projectionMatrix_, &inputStateBlock_.worldViewProjectionMatrix_ );
				hMatrix::inverse( &inputStateBlock_.worldViewMatrix_, &inputStateBlock_.worldViewInverseMatrix_ );
				hMatrix::transpose( &inputStateBlock_.worldViewInverseMatrix_, &inputStateBlock_.worldViewInverseTransposeMatrix_ );
				hMatrix::inverse( &inputStateBlock_.projectionMatrix_, &inputStateBlock_.inverseProjectionMatrix_ );
				hMatrix::mult( &inputStateBlock_.viewMatrix_, &inputStateBlock_.projectionMatrix_, &inputStateBlock_.viewProjectionMatrix_ );
				hMatrix::inverse( &inputStateBlock_.viewProjectionMatrix_, &inputStateBlock_.inverseViewProjectionMatrix_ );
			}

			hUint32 paramCount = pActiveMaterial_->GetShaderParameterCount();
			for ( hUint32 i = 0; i < paramCount; ++i )
			{
				const hShaderParameter* pParam = pActiveMaterial_->GetShaderParameter( i );
				if ( ( pParam->type_ == MATERIALCONSTTYPE_FLOAT || pParam->type_ == MATERIALCONSTTYPE_MATRIX ) &&
					 ( pParam->semanticID_ != EffectSemantics::EFFECTSEMANTICID_MAX || pParam->dirty_ ) )
				{
					pActiveMaterial_->pImpl()->SetFloatArrayParameter( 
						pParam->guidIndex_, pParam->paramData_, pParam->paramSize_/sizeof(float) );
				}

				//set textures
				if ( pParam->type_ == MATERIALCONSTTYPE_TEXTURE && pParam->boundTexture_.HasData() && pParam->dirty_ )
				{
					pActiveMaterial_->pImpl()->SetTextureParameter( i, pParam->boundTexture_->pImpl() );
				}

				pParam->dirty_ = false;
			}

			blendEnable( pActiveMaterial_->AlphaBlendEnabled() );

			if ( pActiveMaterial_->AlphaBlendEnabled() == RSV_ENABLE )
			{
				srcBlend( pActiveMaterial_->AlphaSrcBlend() );
				dstBlend( pActiveMaterial_->AlphaDstBlend() );
				blendFunc( pActiveMaterial_->AlphaBlendFunction() );
			}

			zTest( pActiveMaterial_->ZTestEnabled() );
			zWrite( pActiveMaterial_->ZWriteEnable() );
			zCmpFunc( pActiveMaterial_->ZCompareFunction() );
			setCullMode( pActiveMaterial_->GetCullMode() );
			setFillMode( pActiveMaterial_->GetFillMode() );

			stencilTest( pActiveMaterial_->GetStencilTest() );

			if ( pActiveMaterial_->GetStencilTest() == RSV_ENABLE )
			{
				stencilFail( pActiveMaterial_->GetStencilFail() );
				stencilZFail( pActiveMaterial_->GetStencilZFail() );
				stencilPass( pActiveMaterial_->GetStencilPass() );
				stencilFunc( pActiveMaterial_->GetStencilFunction() );
				stencilRef( pActiveMaterial_->GetStencilRef() );
				stencilWriteMask( pActiveMaterial_->GetStencilWriteMask() );
				stencilMask( pActiveMaterial_->GetStencilMask() );
			}

			SetColourWrite1( pActiveMaterial_->GetColourMask1() );
			SetColourWrite2( pActiveMaterial_->GetColourMask2() );
			SetColourWrite3( pActiveMaterial_->GetColourMask3() );
			SetColourWrite4( pActiveMaterial_->GetColourMask4() );
		}

		for ( hUint32 i = 0; i < nDirtyStates_; ++i )
		{
			Renderer_.SetRenderState( (RENDER_STATE)dirtyStates_[ i ], (RENDER_STATE_VALUE)renderStates_[ dirtyStates_[ i ] ] );
		}

		//apply any extra changes
		if ( pActiveMaterial_ )
		{
			pActiveMaterial_->CommitChanges();
		}

		ClearDirtyStates();
        */
	}

}