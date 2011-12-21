/********************************************************************
	created:	2008/10/24
	created:	24:10:2008   23:18
	filename: 	hlState.cpp
	author:		James Moran
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hStateBase.h"

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
					delete ( hGeneralHeap, child_ );
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
							delete ( hGeneralHeap, subStates_[ i ] );
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
						delete ( hGeneralHeap, child_ );
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
							delete ( hGeneralHeap, subStates_[ i ] );
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
			delete ( hGeneralHeap, pendingChild_ );
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
			delete ( hGeneralHeap, pendingSubStates_[ Id ] );
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
						delete ( hGeneralHeap, child_ );
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
							delete ( hGeneralHeap, subStates_[ i ] );
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