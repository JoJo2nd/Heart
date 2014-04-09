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

#include "precompiled/precompiled.h"
#include "common/ui_id.h"
#include "pkg_viewer.h"

namespace 
{
    ui::ID ID_OPENPKG = ui::marshallNameToID("ID_OPENPKG");
    ui::ID ID_PKGPROPS = ui::marshallNameToID("ID_PKGPROPS");
    ui::ID ID_PKGLIST = ui::marshallNameToID("ID_PKGLIST");
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

PkgViewer::PkgViewer(wxWindow* parent, wxAuiManager* auiManager, wxMenuBar* menuBar) 
    : wxPanel(parent, wxID_ANY, wxDefaultPosition) 
    , auiManager_(auiManager)
{
    wxMenu* file_menu=nullptr;
    auto menu_id = menuBar->FindMenu(uiLoc("&File"));
    if (menu_id != wxNOT_FOUND) {
        file_menu = menuBar->GetMenu(menu_id);
    } else {
        file_menu = new wxMenu();
    }
    file_menu->Append(ID_OPENPKG, uiLoc("Open Package..."));

    auto* menu_parent = menuBar->GetParent();
    if (menu_parent) {
        menu_parent->Bind(wxEVT_COMMAND_MENU_SELECTED, &PkgViewer::onPackageOpen, this, ID_OPENPKG);
    }

    propGrid_ = new wxPropertyGrid(this, ID_PKGPROPS);

    pkgList_ = new wxListView(this, ID_PKGLIST);
    pkgList_->AppendColumn(uiLoc("Loaded Packages"));

    Bind(wxEVT_COMMAND_LIST_ITEM_SELECTED, &PkgViewer::onSelectPackage, this, ID_PKGLIST);


    auto* main_sizer = new wxFlexGridSizer(1, 2, 0, 0);
    main_sizer->AddGrowableCol(0, 2);
    main_sizer->AddGrowableCol(1, 8);
    main_sizer->AddGrowableRow(0);
    main_sizer->Add(pkgList_, 1, wxALL | wxEXPAND);
    main_sizer->Add(propGrid_, 1, wxALL | wxEXPAND);
    main_sizer->SetSizeHints(this);

    SetSizerAndFit(main_sizer);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void PkgViewer::onPackageOpen(wxCommandEvent& event) {
    wxFileDialog openFileDialog(this, uiLoc("Open Package file"), "", "", "PKG files (*.pkg)|*.pkg", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL) {
        return;
    }

    Pkg new_package;
    if (new_package.loadFromFile(openFileDialog.GetPath().c_str())) {
        addPackage(new_package);
        pkgList_->InsertItem(0, openFileDialog.GetPath().c_str());
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void PkgViewer::addPackage(const Pkg& in_pkg) {
    std::shared_ptr<Pkg> pkg(new Pkg);
    *pkg = in_pkg;
    loadedPackages_.push_back(pkg);
    packageMap_.emplace(pkg->filepath_, pkg);
}

void parseMessageToPropertyGrid(const wxString& label, const std::string& entry_name, const google::protobuf::Message& message, wxPropertyGrid* prop_grid, wxPGPropArg parent);

template<typename t_ty>
void addTypeProptery(const t_ty& val, const wxString& label, const wxString& name, wxPropertyGrid* prop_grid, wxPGPropArg parent);

template<typename t_ty>
void propGridAppendIn(t_ty* prop, wxPropertyGrid* prop_grid, wxPGPropArg parent) {
    //prop->ChangeFlag(wxPG_PROP_READONLY, true);
    prop_grid->AppendIn(parent, prop);
}

template<>
void addTypeProptery<google::protobuf::int32>(const google::protobuf::int32& val, const wxString& label,const wxString& name, wxPropertyGrid* prop_grid, wxPGPropArg parent) {
    auto* prop = new wxIntProperty(name, label, val);
    propGridAppendIn(prop, prop_grid, parent);
}

template<>
void addTypeProptery<google::protobuf::int64>(const google::protobuf::int64& val, const wxString& label,const wxString& name, wxPropertyGrid* prop_grid, wxPGPropArg parent) {
    auto* prop = new wxIntProperty(name, label, val);
    propGridAppendIn(prop, prop_grid, parent);
}

template<>
void addTypeProptery<google::protobuf::uint32>(const google::protobuf::uint32& val, const wxString& label,const wxString& name, wxPropertyGrid* prop_grid, wxPGPropArg parent) {
    auto* prop = new wxUIntProperty(name, label, val);
    propGridAppendIn(prop, prop_grid, parent);
}

template<>
void addTypeProptery<google::protobuf::uint64>(const google::protobuf::uint64& val, const wxString& label,const wxString& name, wxPropertyGrid* prop_grid, wxPGPropArg parent) {
    auto* prop = new wxUIntProperty(name, label, val);
    propGridAppendIn(prop, prop_grid, parent);
}

template<>
void addTypeProptery<double>(const double& val, const wxString& label,const wxString& name, wxPropertyGrid* prop_grid, wxPGPropArg parent) {
    auto* prop = new wxFloatProperty(name, label, val);
    propGridAppendIn(prop, prop_grid, parent);
}

template<>
void addTypeProptery<float>(const float& val, const wxString& label,const wxString& name, wxPropertyGrid* prop_grid, wxPGPropArg parent) {
    auto* prop = new wxFloatProperty(name, label, val);
    propGridAppendIn(prop, prop_grid, parent);
}

template<>
void addTypeProptery<bool>(const bool& val, const wxString& label,const wxString& name, wxPropertyGrid* prop_grid, wxPGPropArg parent) {
    auto* prop = new wxBoolProperty(name, label, val);
    propGridAppendIn(prop, prop_grid, parent);
}

template<>
void addTypeProptery<const google::protobuf::EnumValueDescriptor*>(const google::protobuf::EnumValueDescriptor* const& val, const wxString& label, const wxString& name, wxPropertyGrid* prop_grid, wxPGPropArg parent) {
    auto* enum_desc = val->type();
    int values = enum_desc->value_count();
    wxArrayString enum_names;
    wxArrayInt enum_values;
    for (int ei=0, en=values; ei<en; ++ei) {
        auto* enum_value = enum_desc->value(ei);
        enum_names.push_back(enum_value->name().c_str());
        enum_values.push_back(enum_value->number());
    }
    auto* prop = new wxEnumProperty(name, label, enum_names, enum_values, val->number());
    propGridAppendIn(prop, prop_grid, parent);
}

template<>
void addTypeProptery(const std::string& val, const wxString& label, const wxString& name, wxPropertyGrid* prop_grid, wxPGPropArg parent) {
    if (val.length() > 128) {
        auto* prop = new wxLongStringProperty(name, label, val);
        propGridAppendIn(prop, prop_grid, parent);
    } else {
        auto* prop = new wxStringProperty(name, label, val);
        propGridAppendIn(prop, prop_grid, parent);
    }
}

template<>
void addTypeProptery(const google::protobuf::Message& val, const wxString& label, const wxString& name, wxPropertyGrid* prop_grid, wxPGPropArg parent) {
    parseMessageToPropertyGrid(label, name.ToStdString(), val, prop_grid, parent);
}

template<typename t_ty>
void addRepeatedTypeProperty(const t_ty& rep_vals, const wxString& label, const wxString& name, wxPropertyGrid* prop_grid, wxPGPropArg parent) {
    wxString tmp_name = name;
    wxString tmp_label = label;
    for (int i=0, n=rep_vals.size(); i<n; ++i) {
        tmp_name.Printf("%s[%u]", name.c_str(), i);
        tmp_label.Printf("%s.%s[%u]", label.c_str(), name.c_str(), i);
        addTypeProptery(rep_vals.Get(i), tmp_label, tmp_name, prop_grid, parent);
    }
}

void parseMessageToPropertyGrid(const wxString& label, const std::string& entry_name, const google::protobuf::Message& message, wxPropertyGrid* prop_grid, wxPGPropArg parent) {
    using google::protobuf::FieldDescriptor;
    using google::protobuf::UnknownField;
    using google::protobuf::UnknownFieldSet;
    using google::protobuf::EnumValueDescriptor;

    wxString name;
    auto* reflection = message.GetReflection();
    auto* descriptor = message.GetDescriptor();
    uint expected_size = descriptor->field_count();
    std::vector< const FieldDescriptor* > ref_fields; // array of known fields in the message
    UnknownFieldSet unknown_fields;

    ref_fields.reserve(expected_size);

    for (uint i=0, n=descriptor->field_count(); i<n; ++i) {
        auto* field_desc = descriptor->field(i);
        ref_fields.push_back(field_desc);
    }

    name.Printf("%s (%s:%s)", entry_name.c_str(), uiLoc("Type"), message.GetTypeName().c_str());
    auto* this_root = prop_grid->AppendIn(parent, new wxPropertyCategory(name, label));
    wxString field_label = label;

    for(const auto& ref : ref_fields) {
        field_label.Printf("%s.%s", label.c_str(), ref->camelcase_name());
        switch(ref->cpp_type()) {
        case FieldDescriptor::CPPTYPE_INT32  : {
            if (ref->is_repeated()) {
                addRepeatedTypeProperty(reflection->GetRepeatedField<google::protobuf::int32>(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }else {
                addTypeProptery(reflection->GetInt32(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }
        } break;
        case FieldDescriptor::CPPTYPE_INT64  : {
            if (ref->is_repeated()) {
                addRepeatedTypeProperty(reflection->GetRepeatedField<google::protobuf::int64>(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }else {
                addTypeProptery(reflection->GetInt64(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }
        } break;
        case FieldDescriptor::CPPTYPE_UINT32 : {
            if (ref->is_repeated()) {
                addRepeatedTypeProperty(reflection->GetRepeatedField<google::protobuf::uint32>(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }else {
                addTypeProptery(reflection->GetUInt32(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }
        } break;
        case FieldDescriptor::CPPTYPE_UINT64 : {
            if (ref->is_repeated()) {
                addRepeatedTypeProperty(reflection->GetRepeatedField<google::protobuf::uint64>(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }else {
                addTypeProptery(reflection->GetUInt64(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }
        } break;
        case FieldDescriptor::CPPTYPE_DOUBLE : {
            if (ref->is_repeated()) {
                addRepeatedTypeProperty(reflection->GetRepeatedField<double>(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }else {
                addTypeProptery(reflection->GetDouble(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }
        } break;
        case FieldDescriptor::CPPTYPE_FLOAT  : {
            if (ref->is_repeated()) {
                addRepeatedTypeProperty(reflection->GetRepeatedField<float>(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }else {
                addTypeProptery(reflection->GetFloat(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }
        } break;
        case FieldDescriptor::CPPTYPE_BOOL   : {
            if (ref->is_repeated()) {
                addRepeatedTypeProperty(reflection->GetRepeatedField<bool>(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }else {
                addTypeProptery(reflection->GetBool(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }
        } break;
        case FieldDescriptor::CPPTYPE_ENUM   : {
            if (ref->is_repeated()) {
                // Enums are a little akward...
                const std::string& name = ref->camelcase_name();
                const wxString& label = field_label;
                wxString tmp_name = ref->camelcase_name();
                wxString tmp_label = field_label;
                for (int i=0, n=reflection->FieldSize(message, ref); i<n; ++i) {
                    tmp_name.Printf("%s[%u]", name.c_str(), i);
                    tmp_label.Printf("%s.%s[%u]", field_label.c_str(), name.c_str(), i);
                    addTypeProptery(reflection->GetRepeatedEnum(message, ref, i), tmp_label, tmp_name, prop_grid, this_root);
                }
            }else {
                addTypeProptery(reflection->GetEnum(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }
        } break;
        case FieldDescriptor::CPPTYPE_STRING : {
            if (ref->is_repeated()) {
                addRepeatedTypeProperty(reflection->GetRepeatedPtrField<std::string>(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            } else {
                addTypeProptery(reflection->GetString(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }
        } break;
        case FieldDescriptor::CPPTYPE_MESSAGE: {
            if (ref->is_repeated()) {
                addRepeatedTypeProperty(reflection->GetRepeatedPtrField<google::protobuf::Message>(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            } else {
                addTypeProptery(reflection->GetMessage(message, ref), field_label, ref->camelcase_name(), prop_grid, this_root);
            }
        } break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void PkgViewer::onSelectPackage(wxListEvent& event) {
    propGrid_->Clear();
    auto found_itr = packageMap_.find(event.GetLabel().ToStdString());
    if (found_itr != packageMap_.end()) {
        std::string name_label = event.GetLabel().ToStdString();
        parseMessageToPropertyGrid(name_label, name_label, found_itr->second->header_, propGrid_, propGrid_->GetRoot());

        size_t header_size = found_itr->second->headerSize_;
        uint8* base_ptr = found_itr->second->data_.get()+header_size;
        for (int i=0, n=found_itr->second->header_.entries_size(); i<n; ++i) {
            auto& entry = found_itr->second->header_.entries(i);
            google::protobuf::io::ArrayInputStream resourcefilestream(base_ptr+entry.entryoffset(), entry.entrysize());
            google::protobuf::io::CodedInputStream resourcestream(&resourcefilestream);

            Heart::proto::MessageContainer data_container;
            data_container.ParseFromCodedStream(&resourcestream);
            auto* msg = types::createMessageFromName(data_container.type_name().c_str());
            if (msg) {
                msg->ParseFromString(data_container.messagedata());
                wxString resource_label;
                resource_label.Printf("%s%s", name_label.c_str(), entry.entryname().c_str());
                parseMessageToPropertyGrid(resource_label, resource_label.ToStdString(), *msg, propGrid_, propGrid_->GetRoot());
                delete msg;
            }
        }

        propGrid_->CollapseAll();
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool PkgViewer::Pkg::loadFromFile(const char* filepath) {
    std::ifstream fileinput;
    fileinput.open(filepath, std::ios_base::in | std::ios_base::binary);
    if (!fileinput.is_open()) {
        return false;
    }

    fileinput.seekg(0, std::ios_base::end);
    size_t file_len = fileinput.tellg();
    dataLen_ = file_len;
    fileinput.seekg(0, std::ios_base::beg);
    data_.reset(new uint8[file_len], std::default_delete<uint8[]>());
    fileinput.read((char*)data_.get(), file_len);
    fileinput.seekg(0, std::ios_base::beg);

    google::protobuf::io::ArrayInputStream filestream(data_.get(), file_len);
    google::protobuf::io::CodedInputStream resourcestream(&filestream);

    google::protobuf::uint32 headersize;
    resourcestream.ReadVarint32(&headersize);
    headersize += resourcestream.CurrentPosition();
    headerSize_ = headersize;
    auto limit=resourcestream.PushLimit(headersize);
    header_.ParseFromCodedStream(&resourcestream);
    resourcestream.PopLimit(limit);

    filepath_ = filepath;
    return true;
}
