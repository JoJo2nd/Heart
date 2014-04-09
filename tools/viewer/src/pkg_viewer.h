/********************************************************************
    
    Copyright (c) 6:4:2014 James Moran
    
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

class PkgViewer : public wxPanel
{
public:
    PkgViewer(wxWindow* parent, wxAuiManager* auiManager, wxMenuBar* menuBar);
private:

    struct Pkg
    {
        std::string                  filepath_;
        std::shared_ptr<uint8>       data_;
        size_t                       dataLen_;
        Heart::proto::PackageHeader  header_;
        size_t                       headerSize_;

        bool loadFromFile(const char* filepath);
    };

    void addPackage(const Pkg& in_pkg);
    void onPackageOpen(wxCommandEvent& event);
    void onSelectPackage(wxListEvent& event);

    std::vector<std::shared_ptr<Pkg>>           loadedPackages_;
    std::map<std::string, std::shared_ptr<Pkg>> packageMap_;
    wxListView*         pkgList_;
    wxPropertyGrid*     propGrid_;
    wxAuiManager*       auiManager_;
};