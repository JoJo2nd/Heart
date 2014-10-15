/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#if 0
#ifndef HSOUNDMANAGER_H__
#define HSOUNDMANAGER_H__

#include "base/hTypes.h"
#include "base/hLinkedList.h"

namespace Heart
{
    class hSoundSource;

    class  hSoundManager
    {
    public:
        hSoundManager()
        {

        }
        ~hSoundManager()
        {

        }

        void             Initialise();
        void             Update();
        void             CreateChannel( hUint32 /*channelID*/ ) {} //TODO:
        void             DestroyChannel( hUint32 /*channelID*/ ) {} //TODO:
        hSoundSource*    CreateSoundSource( hUint32 channel );
        void             DestroySoundSource( hSoundSource* source );
        void             Destory();

    private:

        hdSoundCore                 impl_;
        hLinkedList< hSoundSource > soundSources_;
    };
}

#endif // HSOUNDMANAGER_H__
#endif