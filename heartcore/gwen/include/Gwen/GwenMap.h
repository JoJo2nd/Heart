/********************************************************************

	filename: 	GwenMap.h	
	
	Copyright (c) 5:5:2012 James Moran
	
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

#ifndef GWENMAP_H__
#define GWENMAP_H__

#include "Gwen/GwenMemory.h"
#include <map>

namespace Gwen
{
    template < class _Kty,
        class _Ty,
        class _Pr = std::less<_Kty> >
    class map_t
    {
    public:
        typedef std::map< _Kty, _Ty, _Pr, GwenSTLAllocator< std::pair<const _Kty, _Ty> > > type;
    };
}

#endif // GWENMAP_H__