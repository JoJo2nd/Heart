/********************************************************************

	filename: 	hStateBase.h	
	
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

#ifndef hlState_h__
#define hlState_h__

namespace Heart
{
	class hStateBase
	{
	public:

		static const hUint32 		MAX_NAME_SIZE = 32;
		static const hUint32		MAX_SUB_STATES = 8;
		static const hUint32 		CONTINUE = 1;
		static const hUint32 		FINISHED = 0;

		hStateBase::hStateBase( const hChar* pstatename )
		{
			strcpy_s( stateName_, MAX_NAME_SIZE, pstatename );
			allFlags_ = 0;
			internalState_ = IS_ENTERING;
			parent_ = NULL;
			child_ = NULL;
			pendingChild_ = NULL;
			for ( hUint32 i = 0; i < MAX_SUB_STATES; ++i )
			{
				subStates_[ i ] = NULL;
				pendingSubStates_[ i ] = NULL;
			}
		}
		virtual						~hStateBase() {}

		const hChar*				Name() const { return &stateName_[0]; }

		// Parent, child & substate access
		hStateBase*					ParentState() { return parent_; }
		hStateBase*					ChildState() { return child_; }
		hStateBase*					SubState( hUint32 id ) { return subStates_[id];}

		// Control functions ( override as necessary )
		virtual void				PreEnter() {}					///< Called once before the repeated calling of Enter()
		virtual hUint32				Enter() { return FINISHED; }	///< Run the entering logic of the state
		virtual void				PostEnter() {}					///< Called once in the transition to Main
		virtual hUint32				Main() = 0;						///< Run the Main state logic
		virtual void				PreLeave() {}					///< Called once in the transition to Leave
		virtual hUint32				Leave() { return FINISHED; }	///< Run the leaving logic of the state
		virtual void				PostLeave() {}					///< Called once after Leave() has finished
		virtual void				Always() {}						///< Always called while the state is active i.e. in Enter(), Main(), or Leave()

		// Render functions ( override as necessary )
		virtual void				EnterRender() {}				///< Rendering function while entering state 
		virtual void				MainRender() {}					///< Rendering function for when the state is current
		virtual void				LeaveRender() {}				///< Rendering function while the state is leaving

		// Main Process & Render (only to be called on the top state)
		virtual hUint32				Process();						///< Main control function. DON'T OVERRIDE UNLESS YOU KNOW WHAT YOU'RE DOING!
		virtual void				Render();						///< Render function. DON'T OVERRIDE UNLESS YOU KNOW WHAT YOU'RE DOING!

		// Callbacks
	// 	virtual void				onChildSpawn();					///< Called as a new child is spawned
	// 	virtual void				onChildActive();				///< Called as a new child becomes active (after child enterOnce)
	// 	virtual void				onChildDying();					///< Called just before a child's leaveOnce
	// 	virtual void				onChildDeath( StateBase* pDeadChild = NULL , hBool IsSubState = hFalse ); ///< Called just as a child is deleted

		// Tell a state to Leave
		virtual void				ForceLeave() { leaveWaiting_ = hTrue; }		///< Force this state to Leave

		// Spawn states
		void						SpawnState( hStateBase* pState ); ///< Call to spawn a child. Causes any current child to Leave before running new state
		void						SpawnSubState( hUint32 Id, hStateBase* pState ); ///< Call to spawn a substate. Causes current substate of same ID to Leave before running new state

		// Cascade states
	// 	void						setCascadeChild( StateBase* pCascadeChild );
	// 	void						setCascadeSubState( hUint32 Id, StateBase* pState );

		//status accessors
		hBool						IsActive() const { return internalState_ != IS_CLEANUP; }

		//
	// 	void						inhibitState( hBool inhibit );
	// 	hBool						isStateInhibited() const;

	protected:

		hUint32									ProcessEnteringState();
		hUint32									ProcessLeavingState();
		hBool									HaveActiveSubStates() const
		{
			for ( hUint32 i = 0; i < MAX_SUB_STATES; ++i )
			{
				if ( subStates_[ i ] )
				{
					return hTrue;
				}
			}

			return hFalse;
		}

		enum InternalState
		{
			IS_ENTERING,
			IS_MAINLOGIC,
			IS_LEAVING,
			IS_CLEANUP,
			IS_MAX
		};

		hChar									stateName_[ MAX_NAME_SIZE ];
		InternalState							internalState_;
		hStateBase*								parent_;
		hStateBase*								child_;
		hArray< hStateBase*, MAX_SUB_STATES >	subStates_;
		hStateBase*								pendingChild_;
		hArray< hStateBase*, MAX_SUB_STATES >	pendingSubStates_;

		union  
		{
			hUint32								allFlags_;
			struct
			{
				hBool							inhibited_ : 1;
				hBool							hasRunPreEnter_ : 1;
				hBool							hasRunEnter_ : 1;
				hBool							hasRunPostEnter_ : 1;
				hBool							hasRunPreLeave_ : 1;
				hBool							hasRunLeave_ : 1;
				hBool							hasRunPostLeave_ : 1;
				hBool							leaveWaiting_ : 1;
			};
		};
	};
}

#endif // hlState_h__