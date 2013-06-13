/********************************************************************

    filename:   mesh_module.h  
    
    Copyright (c) 19:1:2013 James Moran
    
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

class wxCommandEvent;
class MeshExplorerWindow;

typedef unsigned int uint;

enum MeshModuleMenuID
{
    MENUID_SHOWINSPECTOR,
    MENUID_NEW,
    MENUID_LOAD,
    MENUID_SAVE,
    MENUID_EXPORT,
    MENUID_PACKAGEDROPDOWN,
    MENUID_RESNAMEENTRY,
    MENUID_MATERIALBINDINGPICKER,
    MENUID_LODFILEPICKER,
    MENUID_LODLEVELDROPDOWN,
    MENUID_MESHNODELIST,
    MENUID_MATSELECTDROPDOWN,
    MENUID_MATLIST,
    MENUID_ADDLODLEVEL,
    MENUID_YZAXISSWAP,
};

class MeshModule : public vModuleBase
{
public:
    MeshModule();
    ~MeshModule();
    virtual const char*  getModuleName() const { return "Mesh Module"; };
    virtual void initialise(const vModuleInitStruct& initdata);
    virtual void destroy();
    virtual void activate();
    virtual void constantUpdate();
    virtual void activeUpdate();
    virtual void engineUpdate(Heart::hHeartEngine*);
    virtual void engineRender(Heart::hHeartEngine*);
    virtual void deactvate();
private:

    void onShow(wxCommandEvent& evt);

    wxAuiManager*       aui_;
    MeshExplorerWindow* meshExplorer_;
};