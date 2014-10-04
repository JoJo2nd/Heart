/********************************************************************

	filename: 	DeviceSoundUtil.cpp	
	
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