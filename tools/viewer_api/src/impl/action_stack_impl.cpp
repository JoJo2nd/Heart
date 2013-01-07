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

#include "action_stack.h"
#include "boost/smart_ptr.hpp"
#include <stack>

class vActionStack::vImpl
{
public:
    typedef std::stack< boost::shared_ptr< vAction > > ActionStackType;

    size_t  getRedoStackSize() const { return redoStack_.size(); }
    size_t  getUndoStackSize() const { return undoStack_.size(); }
    void    pushAction(vAction* act)
    {
        act->doAction();
        if (act->canUndoRedo()) {
            undoStack_.push(boost::shared_ptr<vAction>(act));
            while(!redoStack_.empty()) redoStack_.pop();
        }
    }
    void    undoAction() 
    {
        if (undoStack_.empty()) return;
        undoStack_.top()->undoAction();
        redoStack_.push(undoStack_.top());
        undoStack_.pop();
    }
    void    redoAction()
    {
        if (redoStack_.empty()) return;
        redoStack_.top()->doAction();
        undoStack_.push(redoStack_.top());
        redoStack_.pop();
    }

    ActionStackType undoStack_;
    ActionStackType redoStack_;
};

vActionStack::vActionStack()
    : impl_(new vImpl)
{
}
vActionStack::~vActionStack()
{
    delete impl_; impl_=NULL;
}
size_t  vActionStack::getRedoStackSize() const
{
    return impl_->getRedoStackSize();
}
size_t  vActionStack::getUndoStackSize() const
{
    return impl_->getUndoStackSize();
}
void    vActionStack::pushAction(vAction* act)
{
    impl_->pushAction(act);
}
void    vActionStack::undoAction()
{
    impl_->undoAction();
}
void    vActionStack::redoAction()
{
    impl_->redoAction();
}