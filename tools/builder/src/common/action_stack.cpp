/********************************************************************

    filename:   action_stack_impl.cpp  
    
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

#include "precompiled/precompiled.h"
#include "common/action_stack.h"
#include <memory>

void vActionStack::pushAction(vAction* act)
{
    act->doAction();
    if (act->canUndoRedo()) {
        undoStack_.push(boost::shared_ptr<vAction>(act));
        while(!redoStack_.empty()) redoStack_.pop();
    }
}
void vActionStack::undoAction() 
{
    if (undoStack_.empty()) return;
    undoStack_.top()->undoAction();
    redoStack_.push(undoStack_.top());
    undoStack_.pop();
}
void vActionStack::redoAction()
{
    if (redoStack_.empty()) return;
    redoStack_.top()->doAction();
    undoStack_.push(redoStack_.top());
    redoStack_.pop();
}