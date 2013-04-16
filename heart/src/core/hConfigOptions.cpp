/********************************************************************

    filename: 	hConfigOptions.cpp
    
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

namespace Heart
{

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    void hConfigOptions::readConfig( const hChar* filename, hIFileSystem* filesystem )
    {
        hIFile* file = filesystem->OpenFileRoot( filename, FILEMODE_READ );
        if ( file ) {
            hUint32 read;
            char* data = (hChar*)hHeapMalloc(GetGlobalHeap(), (hUint32)file->Length());
            read = file->Read(data, (hUint32)file->Length());
            if ( read == file->Length() ) {
                
                if ( doc_.ParseSafe< rapidxml::parse_default >(data, GetGlobalHeap()) ) {
                    hXMLGetter getter(&doc_);
                    readDocToMap(doc_);
                }
            }
            filesystem->CloseFile( file );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    static void buildStringKey(const hXMLGetter& node, hChar* outstr, hUint* bufsize) {
        if (node.parent().ToNode() && node.parent().getNameLen() > 0) {
            buildStringKey(node.parent(), outstr, bufsize);
            hStrCat(outstr, *bufsize, ".");
            *bufsize-=1;
        }
        hStrCat(outstr, *bufsize, node.getName());
        *bufsize-=node.getNameLen();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hConfigOptions::readDocToMap(const hXMLDocument& doc) {
        hXMLGetter node(&doc);
        while (node.ToNode()) {
            if (node.GetValueString() && node.GetValueStringLen() > 0 
            && node.getName() && node.getNameLen() > 0) {
                hUint32 crc;
                hChar namebuf[2048]={0};
                hUint len=(hUint)hStaticArraySize(namebuf);
                buildStringKey(node, namebuf, &len);
                crc=hCRC32::StringCRC(namebuf);
                hOption* op=config_.Find(crc); 
                if (!op) {
                    op=hNEW(GetGlobalHeap(), hOption);
                    config_.Insert(crc, op);
                }
                op->op=node;
            }
            if (node.FirstChild(NULL).ToNode()) {
                node=node.FirstChild(NULL);
            } else if (node.NextSibling().ToNode()) {
                node=node.NextSibling();
            } else {
                while(node.ToNode()) {
                    if (node.parent().NextSibling(NULL).ToNode()) {
                        node=node.parent().NextSibling(NULL);
                        break;
                    } else {
                        node=node.parent();
                    }
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hConfigOptions::getOptionUint(const hChar* key, hUint defval) const {
        hUint32 keycrc=hCRC32::StringCRC(key);
        hOption* op=config_.Find(keycrc);
        if (!op) {
            return defval;
        } else {
            return op->op.GetValueInt(defval);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hInt hConfigOptions::getOptionInt(const hChar* key, hInt defval) const {
        hUint32 keycrc=hCRC32::StringCRC(key);
        hOption* op=config_.Find(keycrc);
        if (!op) {
            return defval;
        } else {
            return op->op.GetValueInt(defval);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hFloat hConfigOptions::getOptionFloat(const hChar* key, hFloat defval) const {
        hUint32 keycrc=hCRC32::StringCRC(key);
        hOption* op=config_.Find(keycrc);
        if (!op) {
            return defval;
        } else {
            return op->op.GetValueFloat(defval);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hConfigOptions::getOptionBool(const hChar* key, hBool defval) const {
        hUint32 keycrc=hCRC32::StringCRC(key);
        hOption* op=config_.Find(keycrc);
        if (!op) {
            return defval;
        } else {
            return hStrICmp(op->op.GetValueString(defval ? "true" : "false"), "true")==0;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hChar* hConfigOptions::getOptionStr(const hChar* key, const hChar* defval) const {
        hUint32 keycrc=hCRC32::StringCRC(key);
        hOption* op=config_.Find(keycrc);
        if (!op) {
            return defval;
        } else {
            return op->op.GetValueString(defval);
        }
    }

}