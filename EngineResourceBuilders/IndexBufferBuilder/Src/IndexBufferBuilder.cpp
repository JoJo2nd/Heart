/********************************************************************

	filename: 	MeshBuilder.cpp	
	
	Copyright (c) 27:3:2011 James Moran
	
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

#include "IndexBufferBuilder.h"
#include "ResourceFileSystem.h"
#include "IndexBuffer.h"
#include "DeviceD3D9IndexBuffer.h"
#include "NvTriStrip.h"

IndexBufferBuilder::IndexBufferBuilder()
{

}

IndexBufferBuilder::~IndexBufferBuilder()
{

}

hBool IndexBufferBuilder::BuildResource()
{
	meshDef_.SetCondenseWhiteSpace( false );
	if ( meshDef_.LoadFile( inputFilename_.c_str(), TIXML_ENCODING_UTF8 ) )
	{
		meshDef_.Accept( this );
	}
	else
	{
		ThrowFatalError( "Index Buffer Builder could not parse input mesh def xml %s.Error:"
			"%s(line:%d column:%d)", inputFilename_.c_str(), meshDef_.ErrorDesc(), meshDef_.ErrorRow(), meshDef_.ErrorCol() );
	}

	//Tri Strip the found indices
	PrimitiveGroup* pPrimGroups = NULL;
	hUint16 nGroups = 0;
	//Guess at the GPU cache size
	//SetCacheSize( 64 );
	SetListsOnly( true );
	GenerateStrips( &indices_[0], indices_.size(), &pPrimGroups, &nGroups, true );

	if ( nGroups > 1 )
	{
		ThrowFatalError( "Tri-Stripping produced more that one primitive group. This isn't supported yet." );
	}

	WriteResourceData( pPrimGroups, nGroups );

	return hTrue;
}

bool IndexBufferBuilder::VisitExit( const TiXmlElement& element )
{
	if ( strcmp( element.Value(), "indices" ) == 0 )
	{
		indices_.resize( elementStack_.size()*3 );
		hUint32 idx = (elementStack_.size()*3)-1;
		
		// parse whats on the element stack
		while( !elementStack_.empty() )
		{
			hUint16 i1,i2,i3;
			if( sscanf_s( elementStack_.top()->GetText(), " %hu %hu %hu ", &i1, &i2, &i3 ) != 3 )
			{
				ThrowFatalError( "Failed to parse face indices" );
			}
			
			indices_[idx--] = i3;
			indices_[idx--] = i2;
			indices_[idx--] = i1;

			elementStack_.pop();
		}
	}
	else if ( strcmp( element.Value(), "face" ) == 0 )
	{
		elementStack_.push( &element );
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void IndexBufferBuilder::WriteResourceData( const PrimitiveGroup* pPrimGroups, hUint16 nGroups )
{
	ResourceDataPtr< Heart::Render::IndexBuffer > ibuffer;
	setBufferSize( 0, sizeof( Heart::Render::IndexBuffer ) );
	getDataFromBuffer( sizeof( Heart::Render::IndexBuffer ), ibuffer );

	hUint32 nIndices;
	hUint16* indices;
	if ( pPrimGroups )
	{
		nIndices = pPrimGroups[0].numIndices;
		indices = pPrimGroups[0].indices;
		ibuffer->pOwner_ = NULL;
		switch ( pPrimGroups[0].type )
		{
		case PT_LIST:
			ibuffer->primitiveType_ = Heart::Render::PRIMITIVETYPE_TRILIST;
			break;
		case PT_STRIP:
			ibuffer->primitiveType_ = Heart::Render::PRIMITIVETYPE_TRISTRIP;
			break;
		case PT_FAN:
			ibuffer->primitiveType_ = Heart::Render::PRIMITIVETYPE_TRIFAN;
			break;
		default:
			ThrowFatalError( "Unknown Primitive Type" );
			break;
		}
	}
	else
	{
		nIndices = indices_.size();
		indices = &indices_[0];
		ibuffer->primitiveType_ = Heart::Render::PRIMITIVETYPE_TRILIST;
	}

	//Create space for pimpls just behind the texture class
	ResourceDataPtr< Heart::Device::D3D9IndexBuffer > pImplData;
	getDataFromBuffer( sizeof( Heart::Device::D3D9IndexBuffer ), pImplData );

	pImplData->pBuffer_ = NULL;
	pImplData->maxIndices_ = (hUint16)nIndices;
	pImplData->nIndices_ = (hUint16)nIndices;
	pImplData->mode_ = 0;

	//Copy out the index data
	ResourceDataPtr< hUint16 > indicesOut;
	ibuffer->pIndices_ = getDataFromBuffer( sizeof( hUint16 )*nIndices, indicesOut );

	for ( hUint32 i = 0, e = nIndices; i != e; ++i )
	{
		indicesOut.GetElement( i ) = indices[i];
	}
}
