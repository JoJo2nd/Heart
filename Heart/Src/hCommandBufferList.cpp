/********************************************************************
	created:	2010/10/02
	created:	2:10:2010   21:51
	filename: 	CommandBufferList.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hCommandBufferList.h"
#include "hRenderCommon.h"

namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hCommandBufferList::~hCommandBufferList()
	{
		CleanUpCommandList();

		//free memory
		delete[] pCommands_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hCommandBufferList::AllocCommandSpace( hUint32 needed )
	{
		hcAssert( needed < MIN_ALLOC_SIZE );

		if ( size_ + needed > allocSize_ )
		{
			if ( pCommands_ )
			{
				hUint32 endOffset = (hUint32)pCommandsEnd_ - (hUint32)pCommands_;
				allocSize_ += MIN_ALLOC_SIZE;
				pCommands_ = (hByte*)hRendererHeap.realloc( pCommands_, allocSize_ );
				pCommandsEnd_ = pCommands_ + endOffset;
			}
			else
			{
				pCommands_ = new ( hRendererHeap ) hByte[ MIN_ALLOC_SIZE ];
				pCommandsEnd_ = pCommands_;
				size_ = 0;
				allocSize_ = MIN_ALLOC_SIZE;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hCommandBufferList::CleanUpCommandList()
	{
		//call destructors
		Cmd::RenderCmdBase* pCmd = (Cmd::RenderCmdBase*)pCommands_;
		while ( pCmd && pCmd != (Cmd::RenderCmdBase*)pCommandsEnd_ )
		{
			hUint32 size = pCmd->size_;
			pCmd->~RenderCmdBase();
			pCmd = (Cmd::RenderCmdBase*)(((hByte*)pCmd) + size);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hCommandBufferList::DoCommandList( hRenderer* pRenderer )
	{
		Cmd::RenderCmdBase* pCmd = (Cmd::RenderCmdBase*)pCommands_;
		while ( pCmd != (Cmd::RenderCmdBase*)pCommandsEnd_ )
		{
			hUint32 size = pCmd->size_;
			pCmd->Execute( pRenderer );
			pCmd = (Cmd::RenderCmdBase*)(((hByte*)pCmd) + size);
		}
	}

}
