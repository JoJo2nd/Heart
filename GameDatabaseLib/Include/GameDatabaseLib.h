/********************************************************************

	filename: 	GameDatabaseLib.h	
	
	Copyright (c) 25:10:2011 James Moran
	
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

#ifndef GAMEDATABASELIB_H__
#define GAMEDATABASELIB_H__

//SDK headers
#include <windows.h>

//STD include headers
#include <assert.h>
#include <memory.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

//Boost inlcude headers
//#include <boost/archive/text_oarchive.hpp>
//#include <boost/archive/text_iarchive.hpp>
#include "boost/filesystem.hpp"
#include "boost/interprocess/mapped_region.hpp"
#include "boost/interprocess/file_mapping.hpp"
#include "boost/archive/xml_oarchive.hpp"
#include "boost/archive/xml_iarchive.hpp"
#include "boost/archive/xml_archive_exception.hpp"
#include "boost/serialization/map.hpp"
#include "boost/serialization/list.hpp"
#include "boost/serialization/vector.hpp"

#include "cryptoCRC32.h"
#include "cryptoMD5.h"

#include "hSerialiser.h"

#include "gdGameDatabaseCommon.h"
#include "gdFileHandle.h"
#include "gdParameters.h"
#include "gdGameDatabaseDLLInterface.h"
#include "gdErrorHandling.h"
#include "gdUniqueResourceID.h"
#include "gdResourceInfo.h"
#include "gdResourceBuilderBase.h"
#include "gdGameDatabaseObject.h"

#endif // GAMEDATABASELIB_H__