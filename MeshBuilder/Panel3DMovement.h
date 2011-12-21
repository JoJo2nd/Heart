/********************************************************************

	filename: 	Panel3DMovement.h	
	
	Copyright (c) 4:3:2011 James Moran
	
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

#ifndef PANEL3DMOVEMENT_H__
#define PANEL3DMOVEMENT_H__

#include "wx/wx.h"
#include "wx/event.h"
#include "wx/kbdstate.h"
#include "Direct3D9.h"
#include "wx/mousestate.h"

namespace UI
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class Panel3DMovement : public wxPanel 
	{
	public:
		Panel3DMovement( wxWindow* parent ) :
			wxPanel( parent, wxID_ANY, wxPoint( 0, 0 ), wxSize(640, 480) )
			,pDevice_( NULL )
			,view_( NULL )
		{
			Bind( wxEVT_ENTER_WINDOW, &Panel3DMovement::OnMouseEnter, this );
			Bind( wxEVT_LEAVE_WINDOW, &Panel3DMovement::OnMouseLeave, this );
			Bind( wxEVT_MOTION, &Panel3DMovement::OnMouseMove, this );
			Bind( wxEVT_KEY_DOWN, &Panel3DMovement::OnKeyDown, this );
			Bind( wxEVT_KEY_UP, &Panel3DMovement::OnKeyUp, this );
			Bind( wxEVT_SIZE, &Panel3DMovement::OnSize, this );
			altDown_ = false;
			rect_.SetWidth( 640 );
			rect_.SetHeight( 480 );
		}

		void SetDevice( Direct3D::Device* pDevice, Rendering::Viewport* view )
		{
			pDevice_ = pDevice;
			view_ = view;
		}
		u32 GetWidth() { return rect_.GetWidth(); }
		u32 GetHeight() { return rect_.GetHeight(); }

	private:
		void OnSize( wxSizeEvent& evt )
		{
			wxSize s = evt.GetSize();
			if ( view_ )
			{
				view_->Resize( s.x, s.y );
			}
		}
		void OnMouseEnter( wxMouseEvent& evt )
		{
			wxClientDC dc(this);
			prevMousePos_ = evt.GetLogicalPosition( dc );
			altDown_ = evt.AltDown();
		}
		void OnMouseLeave( wxMouseEvent& evt )
		{
			//altDown_ = evt.AltDown();
		}
		void OnKeyDown( wxKeyEvent& evt )
		{
			if ( evt.GetKeyCode() == WXK_ALT )
			{
				altDown_ = true;
			}

		}
		void OnKeyUp( wxKeyEvent& evt )
		{
			if ( evt.GetKeyCode() == WXK_ALT )
			{
				altDown_ = false;
			}
		}
		void OnMouseMove( wxMouseEvent& evt )
		{
			wxClientDC dc(this);
			wxPoint pos = evt.GetLogicalPosition( dc );
			if ( altDown_ )
			{
				wxPoint delta = prevMousePos_ - pos;

				if ( evt.LeftIsDown() )
				{
					float dx = fabs( -delta.x/4.0f ) < 0.25f ? 0.0f :-delta.x/4.0f;
					float dy = fabs( -delta.y/4.0f ) < 0.25f ? 0.0f :-delta.y/4.0f;
					pDevice_->RotateView( dy, dx );
				}
				else if ( evt.RightIsDown() )
				{
					float zoom = -delta.y/4.0f ;
					zoom *= pDevice_->GetViewZoom()*0.05f;
					pDevice_->ZoomView( zoom );
				}
				else if ( evt.MiddleIsDown() )
				{
					D3DXVECTOR3 t( delta.x/4.0f, delta.y/4.0f, 0.0f );
					t *= pDevice_->GetViewZoom()*0.002f;
					pDevice_->TranslateView( &t );
				}
			}

			prevMousePos_ = pos;
		}

		wxPoint					prevMousePos_;
		Direct3D::Device*		pDevice_;
		Rendering::Viewport*	view_;
		bool					altDown_;
		wxRect					rect_;
	};
}

#endif // PANEL3DMOVEMENT_H__
