/********************************************************************

	filename: 	hLODGroup.h	
	
	Copyright (c) 13:5:2012 James Moran
	
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

#ifndef HLODGROUP_H__
#define HLODGROUP_H__

namespace Heart
{
    struct hGeomLODLevel
    {
        hVector< hGeometryObject >  renderObjects_;
        hFloat                      minRange_;
    };

    class hLODGroup
    {
    public:
        static const hUint32 MAX_LOD_LEVEL = 9;

        hLODGroup() {}
        ~hLODGroup() {}
    
        hGeomLODLevel*  GetLODLevel( hFloat dist );
        hAABB           GetBounds() const { return totalBounds_; }
    private:

        hArray< hGeomLODLevel, MAX_LOD_LEVEL >      lodLevels_;
        hAABB                                       totalBounds_;
    };
}

#endif // HLODGROUP_H__