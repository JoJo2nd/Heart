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


namespace Heart
{
    struct hInputAction
    {
        hStringID   actionName_;
        hUint       deviceID_;
        hInt16      value_;
        hUint       isRepeat_ : 1;
        hUint       isAxis_ : 1;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    class HEART_DLLEXPORT hActionManager
    {
    public:

        hActionManager() {}
        ~hActionManager() {}

        hBool       initialise(hdSystemWindow* window);
        void        destory();
        void        update();
        void        endOfFrameUpdate();
        void        setDefaultActionMapping(hStringID keyname, hStringID actionname);
        void        clearDefaultActionMapping(hStringID keyname);

    private:

        struct hInputID {
            enum hType {
                hType_Keyboard = 0,
                hType_Mouse = 1,
                hType_Controller = 2,
            };
            // bits 0-1: device type
            // bits 2-11: scan code empty on non-keyboard devices
            union {
                hUint32 id_; 
                struct {
                    hUint type_ : 2; // 0 = keyboard, 1 = mouse, 2 = controller
                    hUint scancode_ : 9; // only valid on keyboard
                    hUint buttonId_ : 8; // only on mouse/controller, axisId is zero
                    hUint axisId_ : 8; // only on mouse/controller, buttonId is zero
                };
            };

            hInputID() {}
            hInputID(hType type, hUint scancode, hUint buttonid, hUint axisid)
                : type_(type), scancode_(scancode), buttonId_(buttonid), axisId_(axisid)
            {}

            hBool operator == (const hInputID& rhs) const { return id_ == rhs.id_; }
            hBool operator != (const hInputID& rhs) const { return id_ != rhs.id_; }
            hBool operator < (const hInputID& rhs) const { return id_ < rhs.id_; }
        };

        struct hInputIDHash
        {
            hSize_t operator()(const hInputID& rhs) const { return rhs.id_; }
        };

        void handleSysEvent(hUint syseventid, const hSysEvent* sysevent);

        typedef std::unordered_map<hInputID, hStringID, hInputIDHash> hActionMapping;
        typedef std::unordered_map<hStringID, hInputID, hStringIDHash> hKeyNameIDMapping;
        typedef std::vector<hInputAction> hActionArray;

        hdSystemWindow*     systemWindow_;
        hKeyNameIDMapping   keyNameToIDMappings_;
        hActionMapping      defaultMappings_;
        hActionArray        bufferedActions_;
    };
}
#endif // HACTIONMANAGER_H__
