/********************************************************************

	filename: 	CreateTextureTest.cpp	
	
	Copyright (c) 30:7:2011 James Moran
	
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

#include "CreateTextureTest.h"


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CreateTextureTest::PostEnter()
{
/*
	engine_->GetRenderer()->CreateTexture( texture_, 256, 256, 1, Heart::TFORMAT_ARGB8, "TestTexture" );

	Heart::hTextureMapInfo lock;
	texture_->Lock( 0, &lock );

	hUint32* pixel = (hUint32*)lock.ptr_;
	for ( hUint32 i = 0; i < 256; ++i )
	{
		for ( hUint32 i2 = 0; i2 < 256; ++i2 )
		{
			if ( i < 128 )
			{
				pixel[i2] = i2 < 128 ? Heart::hColour( 1.0f, 0.0f, 0.0f, 1.0f ) : Heart::hColour( 0.0f, 1.0f, 0.0f, 1.0f );
			}
			else
			{
				pixel[i2] = i2 < 128 ? Heart::hColour( 0.0f, 0.0f, 1.0f, 1.0f ) : Heart::hColour( 1.0f, 1.0f, 1.0f, 1.0f );
			}
		}
		pixel += lock.pitch_/sizeof(hUint32);
	}

	texture_->Unlock( &lock );
*/
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 CreateTextureTest::Main()
{
	return Heart::hStateBase::CONTINUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CreateTextureTest::MainRender()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 CreateTextureTest::Leave()
{
	texture_.Release();
	return Heart::hStateBase::FINISHED;
}
