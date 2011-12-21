/********************************************************************

	filename: 	ActionStack.h	
	
	Copyright (c) 17:1:2011 James Moran
	
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

#ifndef ACTIONSTACK_H__
#define ACTIONSTACK_H__

#include "stdafx.h"
#include <queue>
#include <stack>

namespace Action
{
	enum ActionState
	{
		AS_CONTINUE,
		AS_COMPLETE,

		AS_MAX
	};

	class ICommand
	{
	public:
		virtual ~ICommand() {};
		virtual void Execute() = 0;
		virtual void Undo() = 0;
		virtual void OnClear() {};
	};

	class CommandStack
	{
	public:

		CommandStack() :
			awaitingUndos_( 0 )
			,isUndo_( false )
			,pCurrentCommand_( NULL )
		{

		}
		~CommandStack()
		{

		}

		void	PushCommand( ICommand* pCmd )
		{
			pendingCommands_.push( pCmd );
			Tick();
		}
		void	UndoLastCommand()
		{
			if ( awaitingUndos_ < (completedCommands_.size()+pendingCommands_.size()) )
			{
				++awaitingUndos_;
			}
		}
		void	Clear()
		{
			//TODO:
		}
		void	Tick()
		{
			//if we are running a command, complete it first
			while( pendingCommands_.size() > 0 || pCurrentCommand_ )
			{
				if ( pCurrentCommand_ )
				{
					if ( isUndo_ )
					{
						pCurrentCommand_->Undo();
						delete pCurrentCommand_;
						pCurrentCommand_ = NULL;
					}
					else
					{
						pCurrentCommand_->Execute();
						completedCommands_.push( pCurrentCommand_ );
						pCurrentCommand_ = NULL;
					}
				}
				//Now we can deal with any undo'
				else
				{
					do
					{
						//Any waiting undos? deal with them first
						if ( awaitingUndos_ > 0 )
						{
							pCurrentCommand_ = completedCommands_.top();
							completedCommands_.pop();
							--awaitingUndos_;
						}
						//if there are commands to run, do the first one 
						else if ( pendingCommands_.size() > 0 )
						{
							pCurrentCommand_ = pendingCommands_.front();
							pendingCommands_.pop();
						}
					}
					while( awaitingUndos_ ); // make sure we complete the undos
				}
			}
		}

	private:

		u32							awaitingUndos_;
		bool						isUndo_;
		ICommand*					pCurrentCommand_;
		std::queue< ICommand* >		pendingCommands_;
		std::stack< ICommand* >		completedCommands_;

	};
}

#endif // ACTIONSTACK_H__