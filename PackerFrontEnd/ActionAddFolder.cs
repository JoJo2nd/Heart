using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace PackerFrontEnd
{
    class ActionAddFolder : IAction
    {
        public ActionAddFolder( String folderName, TreeNode parent, TreeView treeView )
        {
            folderName_ = folderName;
            treeView_ = treeView;
            parent_ = parent;
        }

        public override void DoAction()
        {
            addedNode_ = new TreeNode();
            addedNode_.Text = folderName_;
            addedNode_.ImageIndex = 0;

            if (parent_ == null)
            {
                treeView_.Nodes.Add(addedNode_);
            }
            else
            {
                parent_.Nodes.Add(addedNode_);
            }
        }

        public override void UndoAction()
        {
            addedNode_.Remove();
        }

        private String folderName_;
        private TreeView treeView_;
        private TreeNode parent_;
        private TreeNode addedNode_;
    }
}
