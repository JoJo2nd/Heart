/********************************************************************

	filename: 	hConfigOptions.h
	
	Copyright (c) 2011/06/17 James Moran
	
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
#pragma once
#include "tinyxml\tinyxml.h"

#ifndef _CONFIGOPTIONS_H__
#define _CONFIGOPTIONS_H__

namespace Heart
{
	class hIFileSystem;

	class hConfigOptions
	{
	public:
		hConfigOptions() 
		{}
		~hConfigOptions()
		{}

		void		ReadConfig( const hChar* filename, hIFileSystem* filesystem );
		hBool		GetFullscreen() const { return fullscreen_; }
		hBool		GetVsync() const { return vsync_; }
		hUint32		GetScreenWidth() const { return width_; }
		hUint32		GetScreenHeight() const { return height_; }

	private:
		
		hBool			fullscreen_;
		hUint32			width_;
		hUint32			height_;
		hBool			vsync_;
		TiXmlDocument	doc_;

	};
}

#endif // _CONFIGOPTIONS_H__