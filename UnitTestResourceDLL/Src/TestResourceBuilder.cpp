/********************************************************************

	filename: 	TestResourceBuilder.cpp	
	
	Copyright (c) 26:11:2011 James Moran
	
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

#include "TestResourceBuilder.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TestResourceBuilder::TestResourceBuilder( const GameData::gdResourceBuilderConstructionInfo& resInfo )
    : gdResourceBuilderBase( resInfo )
{
    test_.data_ = NULL;
    test_.len_ = 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TestResourceBuilder::~TestResourceBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TestResourceBuilder::BuildResource()
{
    GameData::gdFileHandle* iFile = GetInputFile();

    test_.len_ = (unsigned long)iFile->GetFileSize();
    test_.data_ = new char[ test_.len_ ];

    iFile->Read( test_.data_, test_.len_ );

    GetSerilaiseObject()->Serialise( GetOutputFile(), test_ );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TestResourceBuilder::CleanUpFromBuild()
{
    delete test_.data_;
    test_.data_ = NULL;
}
