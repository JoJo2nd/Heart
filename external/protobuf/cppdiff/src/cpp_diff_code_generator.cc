

#include "cpp_diff_code_generator.h"

#include <memory>

#ifdef _MSC_VER
#   pragma warning (push)
#   pragma warning (disable:4267)
#endif
#include <google/protobuf/testing/file.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/stubs/substitute.h>
#include <google/protobuf/stubs/stl_util.h>
#ifdef _MSC_VER
#   pragma warning (pop)
#endif
#include <iostream>
#include <algorithm>
#include <sstream>


namespace google {
namespace protobuf {
namespace compiler {

static const char* getLabelString(FieldDescriptor::Label l) {
    if (l == FieldDescriptor::LABEL_OPTIONAL) return "optional";
    else if (l == FieldDescriptor::LABEL_REPEATED) return "repeated";
    else if (l == FieldDescriptor::LABEL_REQUIRED) return "required";
    return "";
}

CppDiffGenerator::CppDiffGenerator()
{}

CppDiffGenerator::~CppDiffGenerator() {}


string CppDiffGenerator::GetFullClassName(const string& class_name) {
    string out_classname = class_name;
    size_t loc = 0;
    while (loc = out_classname.find(".", loc)) {
        if (loc == string::npos) {
            break;
        }
        out_classname.replace(loc, 1, "::");
    }
    return out_classname;
}

vector<string> CppDiffGenerator::GetPackageNamespaces(const string& package) {
    vector<string> r;
    string tmp = package;
    size_t loc = 0, prev = 0;
    while (loc = tmp.find(".", prev)) {
        r.push_back(tmp.substr(prev, loc-prev));
        if (loc == string::npos) {
            break;
        }
        prev = loc+1;
    }
    return r;
}

string CppDiffGenerator::GetPackageNamespacePrefix(const string& package) {
    string r;
    auto namespaces = GetPackageNamespaces(package);
    for (const auto& i : namespaces) {
        r += i;
        r += "::";
    }
    return r;
}

string CppDiffGenerator::GetOutputFileName(const string& generator_name, const FileDescriptor* file) {
    return GetOutputFileName(generator_name, file->name());
}

string CppDiffGenerator::GetOutputFileName(const string& generator_name, const string& file) {
	auto newfilename = file;
	newfilename.replace(newfilename.find(".proto"), string::npos, generator_name);
    return newfilename;
}

bool CppDiffGenerator::Generate(
    const FileDescriptor* file,
    const string& parameter,
    GeneratorContext* context,
    string* error) const {

    {
    std::unique_ptr<io::ZeroCopyOutputStream> output(context->Open(GetOutputFileName(".pb.diff.h", file)));
    string package = file->package();
    std::replace(package.begin(), package.end(), '.', '_');
    map<string, string> parameters;
    parameters["src_filename"] = file->name();
    parameters["filename_base"] = GetOutputFileName("", file);
    parameters["package"] = package;
    parameters["function_prefix"] = string("lua_protobuf_") + package + string("_");
    parameters["todo"] = "!?!fix_me!?!";
    io::Printer printer(output.get(), '$');
    printer.Print(parameters,
        "// Auto generated file. Do not edit (It'll be overwritten anyway) \n"
        "//\n"
        "// source proto file: $src_filename$\n"
        "\n"
        "#pragma once"
        "\n"
        "#include <$filename_base$.pb.h>\n"
        "\n");

    auto namespaces = GetPackageNamespaces(file->package());
    for (const auto& i : namespaces) {
        parameters["namespace"] = i;
        printer.Print(parameters, "namespace $namespace$ {\n");
    }
    printer.Indent();
    for (auto i=0, n=file->message_type_count(); i < n; ++i) {
        auto* message_desc = file->message_type(i);
        string cpp_classname = file->package() + "::" + message_desc->name();
        parameters["message_name"] = message_desc->name();
        parameters["cpp_class"] = GetFullClassName(cpp_classname);
        parameters["todo"] = "!?!fix_me!?!";

        printer.Print(parameters,
            "int $message_name$_diff(const $message_name$& a, const $message_name$& b, $message_name$& c);\n"
            "int $message_name$_merge(const $message_name$& a, const $message_name$& b, $message_name$& c);\n");
	}
    printer.Outdent();
    for (const auto& i : namespaces) {
        printer.Print(parameters, "}\n");
    }

    printer.PrintRaw("\n");
    }
    {
    std::unique_ptr<io::ZeroCopyOutputStream> output(context->Open(GetOutputFileName(".pb.diff.cc", file)));
    io::Printer printer(output.get(), '$');
    string package = file->package();
    std::replace(package.begin(), package.end(), '.', '_');
    map<string, string> parameters;
    parameters["src_filename"] = file->name();
    parameters["filename_base"] = GetOutputFileName("", file);
    parameters["header_filename"] = GetOutputFileName(".pb.diff.h", file);
    parameters["package"] = package;
    parameters["dotted_package"] = file->package();
    parameters["todo"] = "!?!fix_me!?!";
    printer.Print(parameters, "#include \"$header_filename$\"\n");
    for (auto i = 0, n = file->dependency_count(); i < n; ++i) {
        auto* dep_desc = file->dependency(i);
        parameters["dep_header_filename"] = GetOutputFileName(".pb.diff.h", dep_desc->name());
        printer.Print(parameters,
            "#include \"$dep_header_filename$\"\n");
    }
    printer.Print(parameters, "\n");
    auto namespaces = GetPackageNamespaces(file->package());
    for (const auto& i : namespaces) {
        parameters["namespace"] = i;
        printer.Print(parameters, "namespace $namespace$ {\n");
    }
    //printer.Indent();
    for (auto i=0, n=file->message_type_count(); i < n; ++i) {
        auto* message_desc = file->message_type(i);
        string cpp_classname = file->package() + "::" + message_desc->name();
        parameters["message_name"] = message_desc->name();
        parameters["cpp_class"] = GetFullClassName(cpp_classname);
        parameters["todo"] = "!?!fix_me!?!";

        printer.Print(parameters,
            "int $message_name$_diff(const $message_name$& a, const $message_name$& b, $message_name$& c) {\n");
        printer.Indent();
        // each field defined in the message
        for (auto fi = 0, fn = message_desc->field_count(); fi < fn; ++fi) {
            auto* field = message_desc->field(fi);
            parameters["field_name"] = field->name();
            parameters["field_name_lower"] = field->name();
            parameters["field_label"] = getLabelString(field->label());
            parameters["field_type"] = field->cpp_type_name();
            std::transform(parameters["field_name_lower"].begin(), parameters["field_name_lower"].end(), parameters["field_name_lower"].begin(), tolower);
            if (field->label() == FieldDescriptor::LABEL_OPTIONAL || field->label() == FieldDescriptor::LABEL_REQUIRED) {
                if (field->label() == FieldDescriptor::LABEL_REQUIRED) {
                    printer.Print(parameters,
                        "// required fields must be set. Ensure it is set even if logic below doesn't set it.\n"
                        "c.set_$field_name_lower$(a.$field_name_lower$());\n");
                }
                printer.Print(parameters, 
                    "if (!a.has_$field_name_lower$() && b.has_$field_name_lower$()) {\n");
                if (field->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) {
                    printer.Print(parameters, 
                        "  c.set_$field_name_lower$(b.$field_name_lower$());\n");
                } else {
                    auto* type_desc = field->message_type();
                    parameters["other_namespace"] = GetPackageNamespacePrefix(type_desc->file()->package());
                    parameters["other_type"] = type_desc->name();
                    printer.Print(parameters,
                        "  c.mutable_$field_name_lower$()->CopyFrom(b.$field_name_lower$());\n");
                }
                printer.Print(parameters,
                    "} else if (a.has_$field_name_lower$() && b.has_$field_name_lower$()) {\n");
                if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
                    auto* type_desc = field->message_type();
                    parameters["other_namespace"] = GetPackageNamespacePrefix(type_desc->file()->package());
                    parameters["other_type"] = type_desc->name();
                    printer.Print(parameters,
                        "  ::$other_namespace$$other_type$_diff(a.$field_name_lower$(), b.$field_name_lower$(), *c.mutable_$field_name_lower$());\n");
                } else {
                    printer.Print(parameters,
                        "  if (a.$field_name_lower$() != b.$field_name_lower$()) {\n"
                        "    c.set_$field_name_lower$(b.$field_name_lower$());\n"
                        "  }\n");
                }
                printer.PrintRaw("}\n");
            } else if (field->label() == FieldDescriptor::LABEL_REPEATED) {
                auto* type_desc = field->message_type();
                if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
                    parameters["other_namespace"] = GetPackageNamespacePrefix(type_desc->file()->package());
                    parameters["other_type"] = type_desc->name();

                }
                printer.Print(parameters, 
                    "for (int i=0, n=a.$field_name_lower$_size(), n2=b.$field_name_lower$_size(); i < n2; ++i) {\n");
                if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
                    printer.Print(parameters,
                        "::$other_namespace$$other_type$ default_instance;\n"
                        "c.add_$field_name_lower$();\n"
                        "::$other_namespace$$other_type$_diff(i < n ? a.$field_name_lower$(i) : default_instance, b.$field_name_lower$(i), *c.mutable_$field_name_lower$(i));\n");
                } else {
                    printer.Print(parameters,
                        "if (i >= n || a.$field_name_lower$(i) != b.$field_name_lower$(i)) {\n"
                        "  c.add_$field_name_lower$(b.$field_name_lower$(i));\n"
                        "} else {\n"
                        "  c.add_$field_name_lower$(a.$field_name_lower$(i));\n"
                        "}\n");
                }
                printer.Print("}\n");
            }
        }
        printer.Print(parameters, "return 0;\n");
        printer.Outdent();
        printer.Print(parameters, "}\n");
        // Merge function
        printer.Print(parameters,
            "int $message_name$_merge(const $message_name$& a, const $message_name$& b, $message_name$& c) {\n");
        printer.Indent();
        // each field defined in the message
        for (auto fi = 0, fn = message_desc->field_count(); fi < fn; ++fi) {
            auto* field = message_desc->field(fi);
            parameters["field_name"] = field->name();
            parameters["field_name_lower"] = field->name();
            parameters["field_label"] = getLabelString(field->label());
            parameters["field_type"] = field->cpp_type_name();
            std::transform(parameters["field_name_lower"].begin(), parameters["field_name_lower"].end(), parameters["field_name_lower"].begin(), tolower);
            if (field->label() == FieldDescriptor::LABEL_OPTIONAL || field->label() == FieldDescriptor::LABEL_REQUIRED) {
                if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
                    auto* type_desc = field->message_type();
                    parameters["other_namespace"] = GetPackageNamespacePrefix(type_desc->file()->package());
                    parameters["other_type"] = type_desc->name();
                    printer.Print(parameters,
                        "if (b.has_$field_name_lower$()) {\n"
                        "  ::$other_namespace$$other_type$_merge(a.has_$field_name_lower$() ? a.$field_name_lower$() : ::$other_namespace$$other_type$(), b.$field_name_lower$(), *c.mutable_$field_name_lower$());\n"
                        "} else if (a.has_$field_name_lower$()) {\n"
                        "  c.mutable_$field_name_lower$()->CopyFrom(a.$field_name_lower$());\n"
                        "}\n");
                } else {
                    printer.Print(parameters, 
                        "if (b.has_$field_name_lower$()) {\n"
                        "  c.set_$field_name_lower$(b.$field_name_lower$());\n"
                        "} else if (a.has_$field_name_lower$()) {\n"
                        "  c.set_$field_name_lower$(a.$field_name_lower$());\n"
                        "}\n");
                }
            } else if (field->label() == FieldDescriptor::LABEL_REPEATED) {
                auto* type_desc = field->message_type();
                if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
                    parameters["other_namespace"] = GetPackageNamespacePrefix(type_desc->file()->package());
                    parameters["other_type"] = type_desc->name();

                }
                printer.Print(parameters,
                    "for (int i=0, n=a.$field_name_lower$_size(), n2=b.$field_name_lower$_size(); i < n2; ++i) {\n");
                if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
                    printer.Print(parameters,
                        "  c.add_$field_name_lower$();\n"
                        "  ::$other_namespace$$other_type$_merge(i < n ? a.$field_name_lower$(i) : ::$other_namespace$$other_type$(), b.$field_name_lower$(i), *c.mutable_$field_name_lower$(i));\n");
                }
                else {
                    printer.Print(parameters,
                        "  c.add_$field_name_lower$(b.$field_name_lower$(i));\n");
                }
                printer.Print("}\n");
            }
        }
        printer.Outdent();
        printer.Print(parameters,
            "  return 0;\n"
            "}\n");
    }
    //printer.Outdent();
    for (const auto& i : namespaces) {
        printer.Print(parameters, "}\n");
    }

    }
    return true;
}

}  // namespace compiler
}  // namespace protobuf
}  // namespace google
