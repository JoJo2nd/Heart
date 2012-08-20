/********************************************************************

	filename: 	hSoundManager.cpp	
	
	Copyright (c) 4:2:2012 James Moran
	
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
namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSoundManager::Initialise()
    {
        impl_.Initialise();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSoundManager::Update()
    {
        HEART_PROFILE_FUNC();
        for ( hSoundSource* i = soundSources_.GetHead(); i; i = i->GetNext() )
        {
            i->Update();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSoundManager::Destory()
    {
        impl_.Destory();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hSoundSource* hSoundManager::CreateSoundSource( hUint32 channel )
    {
        hSoundSource* sound = hNEW(GetGlobalHeap()/*!heap*/, hSoundSource);
        soundSources_.PushBack( sound );

        return sound;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSoundManager::DestroySoundSource( hSoundSource* source )
    {
        soundSources_.Remove( source );
        hDELETE(GetGlobalHeap()/*!heap*/,source);
    }

}