/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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
