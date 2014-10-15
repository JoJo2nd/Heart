/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "pal/hDeviceSoundUtil.h"

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hChar* GetOpenALErrorString( ALenum ercode )
    {
#define HEART_OPENAL_ERRORCASE( x ) case x: return #x
        switch ( ercode )
        {
        HEART_OPENAL_ERRORCASE( AL_INVALID_NAME );
        HEART_OPENAL_ERRORCASE( AL_INVALID_ENUM );
        HEART_OPENAL_ERRORCASE( AL_INVALID_VALUE );
        HEART_OPENAL_ERRORCASE( AL_INVALID_OPERATION );
        HEART_OPENAL_ERRORCASE( AL_OUT_OF_MEMORY );
        default:
        HEART_OPENAL_ERRORCASE( AL_NO_ERROR );
        }
    }
}
