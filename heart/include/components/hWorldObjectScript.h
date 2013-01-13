/********************************************************************

	filename: 	hWorldObjectScript.h	
	
	Copyright (c) 31:3:2012 James Moran
	
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
#ifndef HWORLDOBJECTSCRIPT_H__
#define HWORLDOBJECTSCRIPT_H__

namespace Heart
{
    class hEntityFactory;
#if 0
    class hWorldScriptObject : public hResourceClassBase
    {
    public:
        hWorldScriptObject()
        {

        }
        ~hWorldScriptObject()
        {
            objectDefMap_.Clear(hFalse);
            objectDefs_.Clear();
            worldObjectInstances_.Clear();
        }

        void                       ParseFromXML(const hXMLDocument& xml, const hEntityFactory* entityFactory, hResourceManager* resManager);
        hUint32                    GetObjectInstanceCount() const { return worldObjectInstances_.GetSize(); }
        hWorldObjectTemplate*      GetWorldObjectType(const hChar* typenamestr ) const { return objectDefMap_.Find(hCRC32::StringCRC(typenamestr)); }
        hUint32                    GetWorldObjectTypeCount() const { return objectDefs_.GetSize(); }


    private:

        typedef hVector< hWorldObjectTemplate >           ObjectDefinitionArray;
        typedef hMap< hUint32, hWorldObjectTemplate >     ObjectDefinitionMap;

        ObjectDefinitionArray   objectDefs_;
        ObjectDefinitionMap     objectDefMap_;
        EntityDefinitionArray   worldObjectInstances_;
    };
#endif
}

#endif // HWORLDOBJECTSCRIPT_H__