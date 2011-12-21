/********************************************************************

	filename: 	ResourceLoadTest.cpp	
	
	Copyright (c) 3:12:2011 James Moran
	
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

#include "ResourceLoadTest.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ResourceLoadTest::PreEnter()
{
    resPack_.AddResourceToPackage( "TEXTURES/TEST_TEXTURE_MAP.TEX", &tex1_ );
    resPack_.AddResourceToPackage( "TEXTURES/NARUTO_TEST.TEX", &tex2_ );
    resPack_.AddResourceToPackage( "EFFECTS/SIMPLE.CFX", &mat1_ );
    resPack_.AddResourceToPackage( "FONTS/SYSTEM.FNT", &font1_ );

    resPack_.BeginPackageLoad( engine_->GetResourceManager() );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ResourceLoadTest::Enter()
{
    if ( resPack_.IsPackageLoaded() )
    {
        resPack_.BeingResourceFind();
        tex1_ = resPack_.GetResource( tex1_ );
        tex2_ = resPack_.GetResource( tex2_ );
        mat1_ = resPack_.GetResource( mat1_ );
        font1_ = resPack_.GetResource( font1_ );
        resPack_.EndResourceFind();
        return Heart::hStateBase::FINISHED;
    }
    
    return Heart::hStateBase::CONTINUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ResourceLoadTest::Main()
{
    return Heart::hStateBase::CONTINUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ResourceLoadTest::MainRender()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ResourceLoadTest::PreLeave()
{
    HEART_RESOURCE_SAFE_RELEASE( tex1_ );
    HEART_RESOURCE_SAFE_RELEASE( tex2_ );
    HEART_RESOURCE_SAFE_RELEASE( mat1_ );
    HEART_RESOURCE_SAFE_RELEASE( font1_ );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ResourceLoadTest::Leave()
{
    return Heart::hStateBase::FINISHED;
}
