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
#ifndef _CONFIGOPTIONS_H__
#define _CONFIGOPTIONS_H__

namespace Heart
{
    class hIFileSystem;

    class hConfigOptions
    {
    public:
        hConfigOptions() 
            : filewatch_(0)
        {
        }
        ~hConfigOptions()
        {
            if (filewatch_) {
                hdEndFilewatch(filewatch_);
            }
        }

        void         readConfig( const hChar* filename, hIFileSystem* filesystem );
        hUint        getOptionUint(const hChar* key, hUint defval) const;
        hInt         getOptionInt(const hChar* key, hInt defval) const;
        hFloat       getOptionFloat(const hChar* key, hFloat defval) const;
        hBool        getOptionBool(const hChar* key, hBool defval) const;
        const hChar* getOptionStr(const hChar* key, const hChar* defval) const;

    private:

        struct hOption : public hMapElement< hUint32, hOption > 
        {
            hOption() {}
            ~hOption() {}

            hXMLGetter op;
        };

        typedef hMap< hUint32, hOption > hConfigMap;

        void configChange(const hChar* watchDir, const hChar* filepath, hdFilewatchEvents fileevent);
        void readDocToMap(const hXMLDocument& doc);
        
        hXMLDocument        doc_;
        hConfigMap          config_;
        hdFilewatchHandle   filewatch_;
    };
}

#endif // _CONFIGOPTIONS_H__