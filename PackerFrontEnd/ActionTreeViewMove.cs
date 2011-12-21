using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace PackerFrontEnd
{
    class ActionTreeViewMove : IAction
    {
        public ActionTreeViewMove( TreeNode newParent, TreeNode toMove, TreeView view )
        {
            newParent_ = newParent;
            toMove_ = toMove;
            treeView_ = view;
        }

        public override void DoAction()
        {
            oldParent_ = toMove_.Parent;
            toMove_.Remove();
            if (newParent_ == null)
            {
                treeView_.Nodes.Add(toMove_);
            }
            else
            {
                newParent_.Nodes.Add(toMove_);
            }
        }

        public override void UndoAction()
        {
            toMove_.Remove();
            if (oldParent_ != null)
            {
                oldParent_.Nodes.Add(toMove_);
            }
            else
            {
                treeView_.Nodes.Add(toMove_);
            }
        }

        private TreeNode newParent_;
        private TreeNode toMove_;
        private TreeNode oldParent_;
        private TreeView treeView_;
    }
}
