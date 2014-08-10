/********************************************************************

	filename: 	hActionManager.h	
	
	Copyright (c) 1:4:2012 James Moran
	
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
#pragma once

#ifndef HACTIONMANAGER_H__
#define HACTIONMANAGER_H__

#include "base/hTypes.h"
#include "base/hStringID.h"

namespace Heart
{
    class hLuaStateManager;

    struct  hInputAction
    {
        hStringID   actionName_;
        hInt64      deviceID_;
        hInt        value_;
        hUint       rising_ : 1; // aka button down
        hUint       falling_ : 1; // aka button up
        hUint       isRepeat_ : 1;
        hUint       isAxis_ : 1;
    };

    class hdSystemWindow;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    class  hActionManager
    {
    public:

        hActionManager();
        ~hActionManager();

        hBool       initialise(hdSystemWindow* window);
        void        destory();
        void        update();
        void        endOfFrameUpdate();
        void        setDefaultActionMapping(hStringID keyname, hStringID actionname);
        void        clearDefaultActionMapping(hStringID keyname);
        void        registerLuaLib(hLuaStateManager* luaStateManager);
        hBool       queryAction(hUint id, hStringID action, hInputAction* value);

    private:
        struct hActionManagerImpl*     impl_;
    };
}
#endif // HACTIONMANAGER_H__
