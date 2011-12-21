using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace PackerFrontEnd
{
    public abstract class IAction
    {
        public abstract void DoAction();
        public abstract void UndoAction();
    }

    public class ActionStack
    {
        private ActionStack()
        {
            actionStack_ = new Stack<IAction>();
        }

        public static ActionStack Instance()
        {
            if ( instance_ == null )
            {
                instance_ = new ActionStack();
            }
            return instance_;
        }

        public void ClearStack()
        {
            //TODO:
        }

        public void PushAction(IAction action)
        {
            action.DoAction();
            actionStack_.Push(action);
        }
        public void PopAction()
        {
            if ( actionStack_.Count > 0 )
            {
                actionStack_.Peek().UndoAction();
                actionStack_.Pop();
            }
        }

        private static ActionStack instance_ = null;

        private Stack<IAction> actionStack_;
    }
}
