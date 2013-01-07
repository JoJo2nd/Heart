/********************************************************************

    filename:   action_stack.h  
    
    Copyright (c) 3:1:2013 James Moran
    
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

#ifndef ACTION_STACK_H__
#define ACTION_STACK_H__

#include "viewer_api_config.h"

class vAction
{
public:
    virtual ~vAction();
    virtual bool    canUndoRedo() = 0;
    virtual void    doAction() = 0;
    virtual void    undoAction() = 0;
};

class VAPI_EXPORT vActionStack
{
    VAPI_PIMPL(vActionStack);
public:
    vActionStack();
    ~vActionStack();
    size_t  getRedoStackSize() const;
    size_t  getUndoStackSize() const;
    void    pushAction(vAction* act);
    void    undoAction();
    void    redoAction();
};

#endif // ACTION_STACK_H__