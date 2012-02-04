/********************************************************************

	filename: 	EntityLibCommon.h	
	
	Copyright (c) 28:1:2012 James Moran
	
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
#ifndef ENTITYLIBCOMMON_H__
#define ENTITYLIBCOMMON_H__

#define _CRT_SECURE_NO_WARNINGS

#include "boost/archive/xml_oarchive.hpp"
#include "boost/archive/xml_iarchive.hpp"
#include "boost/archive/xml_archive_exception.hpp"
#include "boost/serialization/export.hpp"
#include "boost/serialization/map.hpp"
#include "boost/serialization/list.hpp"
#include "boost/serialization/vector.hpp"
#include "boost/serialization/string.hpp"

#include "entitylib/EntityLib.h"

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <assert.h>



#define ENTITYLIB_ERROR( msg )                      ENTITYLIB_ERROR_GENERIC1( msg )
#define ENTITYLIB_ERROR_GENERIC()                   ENTITYLIB_ERROR_GENERIC1( "Generic Error." )
#define ENTITYLIB_ERROR_GENERIC1( m )               ENTITYLIB_ERROR_GENERIC2( m, __FILE__, __LINE__ )
#define ENTITYLIB_ERROR_GENERIC2( m, file, line )   ENTITYLIB_ERROR_GENERIC3( m, file, line )
#define ENTITYLIB_ERROR_GENERIC3( m, file, line )   Entity::IErrorObject( false, m, "\n" file "(" #line ")" )
#define ENTITYLIB_OK()                              Entity::IErrorObject( true, "None", "\nNone" )

#endif // ENTITYLIBCOMMON_H__
