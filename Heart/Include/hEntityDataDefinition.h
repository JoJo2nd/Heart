/********************************************************************

	filename: 	hEntityDataDefinition.h	
	
	Copyright (c) 23:1:2012 James Moran
	
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
#ifndef HENTITYDATADEFINITION_H__
#define HENTITYDATADEFINITION_H__

#include "hTypes.h"
#include "hComponent.h"

namespace Heart
{
    struct hComponentPropertyDefinition
    {
        const hChar*            name_;
        hComponentPropertyType  type_;
        union
        {
            hUint32             uintValue_;
            hInt32              intValue_;
            hUint16             uint16Value_;
            hInt16              int16Value_;
            hFloat              floatValue_;
            hChar*              stringValue_;
            hUint32             resourceIDValue_;
        }values_;
    };

    class hComponentDataDefinition
    {
    public:
        hComponentDataDefinition();
        ~hComponentDataDefinition();

    private:

        hComponentDataDefinition( const hComponentDataDefinition& rhs );
        hComponentDataDefinition& operator = ( const hComponentDataDefinition& rhs );

        hVector< hComponentPropertyDefinition > properties_;
        hChar*                                  name_;
    };

    class hEntityDataDefinition : public hMapElement< hUint32, hEntityDataDefinition >
    {
    public:
        hEntityDataDefinition();
        ~hEntityDataDefinition();

    private:

        hEntityDataDefinition( const hEntityDataDefinition& rhs );
        hEntityDataDefinition& operator = ( const hEntityDataDefinition& rhs );

        hChar*                              entityTypeName_;
        hChar*                              parentEntityName_;
        hEntityDataDefinition*              parentEntity_;
        hVector< hComponentDataDefinition > componentDefinitions_;
    };
}

#endif // HENTITYDATADEFINITION_H__