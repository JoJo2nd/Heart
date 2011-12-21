/********************************************************************

	filename: 	UIPropertyID.h	
	
	Copyright (c) 1:11:2011 James Moran
	
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

#ifndef UIPROPERTYID_H__
#define UIPROPERTYID_H__

#define DEFINE_UIPID( name )\
    extern const pChar* UIPLABEL_##name;\
    extern const pChar* UIPID_##name

#define DECLARE_UIPID( label, name )\
    const pChar* UIPLABEL_##name = label;\
    const pChar* UIPID_##name = #label "_" #name "_ID"

DEFINE_UIPID( RESPARAMS );
DEFINE_UIPID( RESTYPENAME );
DEFINE_UIPID( RESINPUTFILE );


#endif // UIPROPERTYID_H__