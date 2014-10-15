/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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