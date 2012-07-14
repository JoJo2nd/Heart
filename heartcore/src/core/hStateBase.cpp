/********************************************************************

	filename: 	hStateBase.cpp	
	
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


namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hStateBase::Process()
	{
		if ( hasRunPreEnter_ == hTrue )
		{
			Always();
		}

		if ( leaveWaiting_ == hTrue )
		{
			internalState_ = IS_LEAVING;
		}

		if ( internalState_ == IS_ENTERING )
		{
			ProcessEnteringState();
		}
		else if ( internalState_ == IS_MAINLOGIC )
		{
			hUint32 ret = FINISHED;
			if ( child_ )
			{
				ret = child_->Process();
				if ( ret == FINISHED )
				{
					delete ( GetGlobalHeap(), child_ );
					child_ = NULL;
				}
			}
			else
			{
				// Process ourselves [10/26/2008 James]
				if ( Main() == FINISHED )
				{
					internalState_ = IS_LEAVING;
				}

				// do sub states [10/26/2008 James]
				for ( hUint32 i = 0; i < MAX_SUB_STATES; ++i )
				{
					if ( subStates_[ i ] )
					{
						ret = subStates_[ i ]->Process();
						if ( ret == FINISHED )
						{
							delete ( GetGlobalHeap(), subStates_[ i ] );
							subStates_[ i ] = NULL;
						}
					}
				}
			}

			// To Do:deal with spawning states
			if ( pendingChild_ )
			{
				if ( child_ )
				{
					if ( !child_->hasRunPreLeave_ )
					{
						// child has yet to be told to Leave [10/26/2008 James]
						child_->ForceLeave();
					}
					else if ( child_->internalState_ == IS_CLEANUP )
					{
						delete ( GetGlobalHeap(), child_ );
						child_ = pendingChild_;
						child_->parent_ = this;
						pendingChild_ = NULL;
					}
				}
				else
				{
					child_ = pendingChild_;
					child_->parent_;
				}
			}

			for ( hUint32 i = 0; i < MAX_SUB_STATES; ++i )
			{
				if ( pendingSubStates_[ i ] )
				{
					if ( subStates_[ i ] )
					{
						if ( !subStates_[ i ]->hasRunPreLeave_ )
						{
							// child has yet to be told to Leave [10/26/2008 James]
							subStates_[ i ]->ForceLeave();
						}
						else if ( subStates_[ i ]->internalState_ == IS_CLEANUP )
						{
							delete ( GetGlobalHeap(), subStates_[ i ] );
							subStates_[ i ] = pendingChild_;
							subStates_[ i ]->parent_ = this;
							pendingChild_ = NULL;
						}
					}
					else
					{
						subStates_[ i ] = pendingChild_;
						subStates_[ i ]->parent_ = this;
					}
				}
			}
		}
		else if ( internalState_ == IS_LEAVING )
		{
			ProcessLeavingState();
		}

		return internalState_ == IS_CLEANUP ? FINISHED : CONTINUE;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hStateBase::Render()
	{
		if ( internalState_ == IS_ENTERING )
		{
			EnterRender();
		}
		else if ( internalState_ == IS_MAINLOGIC )
		{
			MainRender();
		}
		else if ( internalState_ == IS_LEAVING )
		{
			LeaveRender();
		}

		if ( internalState_ != IS_CLEANUP )
		{
			for ( hUint32 i = 0; i < MAX_SUB_STATES; ++i )
			{
				if ( subStates_[ i ] )
				{
					subStates_[ i ]->Render();
				}
			}
			if ( child_ )
			{
				child_->Render();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hStateBase::SpawnState( hStateBase* pState )
	{
		if ( pendingChild_ )
		{
			hcPrintf( "Child State %s was deleted before it could be processed\n", pendingChild_->Name() );
			delete ( GetGlobalHeap(), pendingChild_ );
		}
		pendingChild_ = pState;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hStateBase::SpawnSubState( hUint32 Id, hStateBase* pState )
	{
		if ( pendingSubStates_[ Id ] )
		{
			hcPrintf( "Sub State %s was deleted before it could be processed\n", pendingSubStates_[ Id ]->Name() );
			delete ( GetGlobalHeap(), pendingSubStates_[ Id ] );
		}
		pendingSubStates_[ Id ] = pState;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hStateBase::ProcessEnteringState()
	{
		if ( !hasRunPreEnter_ )
		{
			PreEnter();
			hasRunPreEnter_ = hTrue;
		}

		if ( !hasRunEnter_ )
		{
			if ( Enter() == FINISHED )
			{
				hasRunEnter_ = hTrue;

				PostEnter();

				hasRunPostEnter_ = hTrue;

				internalState_ = IS_MAINLOGIC;
			}
		}

		return CONTINUE;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hStateBase::ProcessLeavingState()
	{
		if ( !hasRunPreLeave_ )
		{
			if ( child_ || HaveActiveSubStates() )
			{
				// we wait until all sub states and children have finished before we Leave
				if ( child_ )
				{
					if ( child_->internalState_ < IS_LEAVING )
					{
						child_->ForceLeave();
					}
					else if ( child_->internalState_ == IS_CLEANUP )
					{
						delete ( GetGlobalHeap(), child_ );
						child_ = NULL;
					}
				}

				for ( hUint32 i = 0; i < MAX_SUB_STATES; ++i )
				{
					if ( subStates_[ i ] ) 
					{ 
						if ( subStates_[ i ]->internalState_ < IS_LEAVING )
						{
							subStates_[ i ]->ForceLeave();
						}
						else if ( subStates_[ i ]->internalState_ == IS_CLEANUP )
						{
							delete ( GetGlobalHeap(), subStates_[ i ] );
							subStates_[ i ] = NULL;
						}
					}
				}
			}
			else
			{
				PreLeave();
				hasRunPreLeave_ = hTrue;
			}
		}
		else if ( !hasRunLeave_ )
		{
			if ( Leave() == FINISHED )
			{
				hasRunLeave_ = hTrue;
			}
		}
		else if ( !hasRunPostLeave_ )
		{
			PostLeave();
			hasRunPostLeave_ = hTrue;
			internalState_ = IS_CLEANUP;
		}

		return CONTINUE;
	}

}