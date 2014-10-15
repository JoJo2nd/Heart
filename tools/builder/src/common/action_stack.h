/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#ifndef ACTION_STACK_H__
#define ACTION_STACK_H__

#include <stack>

class vAction
{
public:
    virtual ~vAction();
    virtual bool    canUndoRedo() = 0;
    virtual void    doAction() = 0;
    virtual void    undoAction() = 0;
};

class vActionStack
{
public:

    vActionStack() {}
    ~vActionStack() {}
    size_t  getRedoStackSize() const { return redoStack_.size(); }
    size_t  getUndoStackSize() const { return undoStack_.size(); }
    void    pushAction(vAction* act);
    void    undoAction();
    void    redoAction();

private:
    typedef std::stack< boost::shared_ptr< vAction > > ActionStackType;

    ActionStackType undoStack_;
    ActionStackType redoStack_;
};

#endif // ACTION_STACK_H__