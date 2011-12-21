/********************************************************************

	filename: 	PropertiesPane.h	
	
	Copyright (c) 19:8:2011 James Moran
	
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
#ifndef PROPERTIESPANE_H__
#define PROPERTIESPANE_H__

class PropertiesPane : public wxPropertyGrid
{
public:
	PropertiesPane( wxWindow* parent ) 
        : wxPropertyGrid( parent, GetMenuID() )
	{
		instance_ = this;

		defaultPaneInfo_.Dock();
		defaultPaneInfo_.Caption( GetWindowPaneName() );
		defaultPaneInfo_.Name( GetWindowPaneName() );
		defaultPaneInfo_.Show();
		defaultPaneInfo_.MinSize( 300, 300 );
		defaultPaneInfo_.Right();
	}
	~PropertiesPane()
	{

	}

	DEFINE_WINDOW_PANE_INSTANCE( PropertiesPane );

private:

};

#endif // PROPERTIESPANE_H__