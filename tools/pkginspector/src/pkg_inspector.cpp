/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "getopt.h"
#include "type_database.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <fstream>

/*
    Proto buffers spits out a lot of warnings about 64 bit to 32 bit conversions.
    We disable these warnings just around these sections.
*/
#if defined (_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable:4244)
#   pragma warning(disable:4267)
#endif

#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/message.h"
#include "google/protobuf/descriptor.h"
#include "package.pb.h"
#include "resource_common.pb.h"

#if defined (_MSC_VER)
#   pragma warning(pop)
#endif

const char options[] = {"lrao:"};
static struct option long_options[] = {
    {"list-resources",  no_argument,        0, 'l'},
    {"dump-readable",   no_argument,        0, 'r'},
    {"dump-all",        no_argument,        0, 'a'},
    {"output",          required_argument,  0, 'o'},
    {0, 0, 0, 0}
};
enum class Option { // ensure these match options above
    optListResources = 0,
    optDumpReabable ,
    optDumpAll,
    optOutputToFile,
};

static void print_usage() {
    printf("pkginspector: ");
    printf(
        "Usage: pkginspector [options] filepath1 filepath2 ... filepath2\n"
        "Available options are:\n"
        "\tlist-resources         - list resources in pkg\n"
        "\tdump-readable          - print all readable strings within resources in pkg.\n"
        "\tdump-all               - print all information from resources in pkg. Overrides dump-readable.\n"
        "\toutput                 - output to file. Without this option stdout is used.\n"
    );
}

struct Options {
    Options() 
        : listResources_(false)
        , dumpReadable_(false)
        , dumpAll_(false)
    {}
    std::vector<std::string> inputFiles_;
    std::string              outputFile_;
    bool                     listResources_;
    bool                     dumpReadable_;
    bool                     dumpAll_;
};

bool getOptions(int argc, char **argv, Options* out_opts) {
    int c;
    for (;;) {
        /* getopt_long stores the option index here. */
        Option opt;
        int option_index = 0;

        c = gop_getopt_long (argc, argv, options, long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

       switch (c) {
       case 0: opt = (Option)option_index; break;
       case 'l': opt = Option::optListResources; break;
       case 'r': opt = Option::optDumpReabable; break;
       case 'a': opt = Option::optDumpAll; break;
       case 'o': opt = Option::optOutputToFile; break;
       case '?':
           /* getopt_long already printed an error message. */
       default:
           return false;
       }

       switch (opt)
       {
       case Option::optListResources:
           out_opts->listResources_=true;
           break;
       case Option::optDumpReabable:
           out_opts->dumpReadable_=true;
           break;
       case Option::optDumpAll:
           out_opts->dumpAll_=true;
           break;
       case Option::optOutputToFile:
           out_opts->outputFile_ = optarg;
           break;
       default:
           break;
       }
    }

    /* For any remaining command line arguments (not options). */
    while (optind < argc) {
        out_opts->inputFiles_.push_back(argv[optind++]);
    }

    return true;
}

struct Pkg {
    std::vector<unsigned char>   data_;
    Heart::proto::PackageHeader  header_;
    size_t                       headerSize_;
};

bool loadPkgFromFile(const char* filepath, Pkg* outpkg) {
    std::ifstream fileinput;
    fileinput.open(filepath, std::ios_base::in | std::ios_base::binary);
    if (!fileinput.is_open()) {
        return false;
    }

    fileinput.seekg(0, std::ios_base::end);
    size_t file_len = fileinput.tellg();

    if (file_len == 0) {
        return false;
    }

    outpkg->data_.resize(file_len);
    fileinput.seekg(0, std::ios_base::beg);
    fileinput.read((char*)outpkg->data_.data(), file_len);
    fileinput.seekg(0, std::ios_base::beg);

    google::protobuf::io::ArrayInputStream filestream(outpkg->data_.data(), (int)file_len);
    google::protobuf::io::CodedInputStream resourcestream(&filestream);

    google::protobuf::uint32 headersize;
    resourcestream.ReadVarint32(&headersize);
    headersize += resourcestream.CurrentPosition();
    outpkg->headerSize_ = headersize;
    auto limit=resourcestream.PushLimit(headersize);
    outpkg->header_.ParseFromCodedStream(&resourcestream);
    resourcestream.PopLimit(limit);
    return true;
}

template<typename t_ty, bool printarraydim>
struct TypePrinter {
    static void print(const t_ty& val, const std::string& fieldname,  FILE* output, int tabcount = 0, size_t arrayindex = 0);
};

template<typename t_ty>
void printType(const t_ty& val, const std::string& fieldname, FILE* output, int tabcount = 0, size_t arrayindex = 0) {
    TypePrinter<t_ty, false>::print(val, fieldname, output, tabcount, arrayindex);
}

template<typename t_ty>
void printTypeArray(const t_ty& val, const std::string& fieldname, FILE* output, int tabcount = 0, size_t arrayindex = 0) {
    TypePrinter<t_ty, true>::print(val, fieldname, output, tabcount, arrayindex);
}

template< typename t_ty>
void printRepeatedType(const t_ty& rep_vals, const std::string& fieldname, FILE* output, int tabcount) {
    for (int i=0, n=rep_vals.size(); i<n; ++i) {
        TypePrinter<t_ty::value_type, true>::print(rep_vals.Get(i), fieldname, output, tabcount, i);
    }
}

void parseMessageToFILE(const google::protobuf::Message& message, FILE* output, int tabcount = 0) {
    using google::protobuf::FieldDescriptor;
    using google::protobuf::UnknownField;
    using google::protobuf::UnknownFieldSet;
    using google::protobuf::EnumValueDescriptor;

    auto* reflection = message.GetReflection();
    auto* descriptor = message.GetDescriptor();
    size_t expected_size = descriptor->field_count();
    std::vector< const FieldDescriptor* > ref_fields; // array of known fields in the message
    UnknownFieldSet unknown_fields;

    ref_fields.reserve(expected_size);

    for (int i=0, n=descriptor->field_count(); i<n; ++i) {
        auto* field_desc = descriptor->field(i);
        ref_fields.push_back(field_desc);
    }

    fprintf(output, "%s\n", /*std::string(tabcount,' ').c_str(),*/ message.GetTypeName().c_str());
    ++tabcount;
    for(const auto& ref : ref_fields) {
        switch(ref->cpp_type()) {
        case FieldDescriptor::CPPTYPE_INT32 : {
            if (ref->is_repeated()) {
                printRepeatedType(reflection->GetRepeatedField<google::protobuf::int32>(message, ref), ref->camelcase_name(), output, tabcount);
            }else {
                printType(reflection->GetInt32(message, ref), ref->camelcase_name(), output, tabcount);
            }
        } break;
        case FieldDescriptor::CPPTYPE_INT64  : {
            if (ref->is_repeated()) {
                printRepeatedType(reflection->GetRepeatedField<google::protobuf::int64>(message, ref), ref->camelcase_name(), output, tabcount);
            }else {
                printType(reflection->GetInt64(message, ref), ref->camelcase_name(), output, tabcount);
            }
        } break;
        case FieldDescriptor::CPPTYPE_UINT32 : {
            if (ref->is_repeated()) {
                printRepeatedType(reflection->GetRepeatedField<google::protobuf::uint32>(message, ref), ref->camelcase_name(), output, tabcount);
            }else {
                printType(reflection->GetUInt32(message, ref), ref->camelcase_name(), output, tabcount);
            }
        } break;
        case FieldDescriptor::CPPTYPE_UINT64 : {
            if (ref->is_repeated()) {
                printRepeatedType(reflection->GetRepeatedField<google::protobuf::uint64>(message, ref), ref->camelcase_name(), output, tabcount);
            }else {
                printType(reflection->GetUInt64(message, ref), ref->camelcase_name(), output, tabcount);
            }
        } break;
        case FieldDescriptor::CPPTYPE_DOUBLE : {
            if (ref->is_repeated()) {
                printRepeatedType(reflection->GetRepeatedField<double>(message, ref), ref->camelcase_name(), output, tabcount);
            }else {
                printType(reflection->GetDouble(message, ref), ref->camelcase_name(), output, tabcount);
            }
        } break;
        case FieldDescriptor::CPPTYPE_FLOAT  : {
            if (ref->is_repeated()) {
                printRepeatedType(reflection->GetRepeatedField<float>(message, ref), ref->camelcase_name(), output, tabcount);
            }else {
                printType(reflection->GetFloat(message, ref), ref->camelcase_name(), output, tabcount);
            }
        } break;
        case FieldDescriptor::CPPTYPE_BOOL   : {
            if (ref->is_repeated()) {
                printRepeatedType(reflection->GetRepeatedField<bool>(message, ref), ref->camelcase_name(), output, tabcount);
            }else {
                printType(reflection->GetBool(message, ref), ref->camelcase_name(), output, tabcount);
            }
        } break;
        case FieldDescriptor::CPPTYPE_ENUM   : {
            // Enums are a little akward...
            if (ref->is_repeated()) {
                for (int i=0, n=reflection->FieldSize(message, ref); i<n; ++i) {
                    printTypeArray(*reflection->GetEnum(message, ref), ref->camelcase_name(), output, tabcount, i);
                }
            }else {
                printType(*reflection->GetEnum(message, ref), ref->camelcase_name(), output, tabcount);
            }
        } break;
        case FieldDescriptor::CPPTYPE_STRING : {
            //TODO: Output binary as HEX numbers.
            // can't print bytes 
            if (ref->type() == FieldDescriptor::TYPE_BYTES) {
                fprintf(output, "%s%s=<BINARY DATA>\n", std::string(tabcount,' ').c_str(), ref->camelcase_name().c_str());
                break;
            }
            if (ref->is_repeated()) {
                printRepeatedType(reflection->GetRepeatedPtrField<std::string>(message, ref), ref->camelcase_name(), output, tabcount);
            }else {
                printType(reflection->GetString(message, ref), ref->camelcase_name(), output, tabcount);
            }
        } break;
        case FieldDescriptor::CPPTYPE_MESSAGE: {
            if (ref->is_repeated()) {
                printRepeatedType(reflection->GetRepeatedPtrField<google::protobuf::Message>(message, ref), ref->camelcase_name(), output, tabcount);
            }else {
                printType(reflection->GetMessage(message, ref), ref->camelcase_name(), output, tabcount);
            }
        } break;
        }
    }
}

template<bool printarraydim>
struct TypePrinter<google::protobuf::int32, printarraydim> {
    static void print(const google::protobuf::int32& val, const std::string& fieldname, FILE* output, int tabcount, size_t arrayindex) {
        if (printarraydim)
            fprintf(output, "%s%s[%u]=%d\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), arrayindex, val);
        else
            fprintf(output, "%s%s=%d\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), val);
    }
};

template<bool printarraydim>
struct TypePrinter<google::protobuf::int64, printarraydim> {
    static void print(const google::protobuf::int64& val, const std::string& fieldname, FILE* output, int tabcount, size_t arrayindex) {
        if (printarraydim)
            fprintf(output, "%s%s[%u]=%lld\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), arrayindex, val);
        else
            fprintf(output, "%s%s=%lld\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), val);
    }
};

template<bool printarraydim>
struct TypePrinter<google::protobuf::uint32, printarraydim> {
    static void print(const google::protobuf::uint32& val, const std::string& fieldname, FILE* output, int tabcount, size_t arrayindex) {
        if (printarraydim)
            fprintf(output, "%s%s[%u]=%u\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), arrayindex, val);
        else
            fprintf(output, "%s%s=%u\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), val);
    }
};

template<bool printarraydim>
struct TypePrinter<google::protobuf::uint64, printarraydim> {
    static void print(const google::protobuf::uint64& val, const std::string& fieldname, FILE* output, int tabcount, size_t arrayindex) {
        if (printarraydim)
            fprintf(output, "%s%s[%u]=%llu\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), arrayindex, val);
        else
            fprintf(output, "%s%s=%llu\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), val);
    }
};

template<bool printarraydim>
struct TypePrinter<float, printarraydim> {
    static void print(const float& val, const std::string& fieldname, FILE* output, int tabcount, size_t arrayindex) {
        if (printarraydim)
            fprintf(output, "%s%s[%u]=%g\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), arrayindex, val);
        else
            fprintf(output, "%s%s=%g\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), val);
    }
};

template<bool printarraydim>
struct TypePrinter<double, printarraydim> {
    static void print(const double& val, const std::string& fieldname, FILE* output, int tabcount, size_t arrayindex) {
        if (printarraydim)
            fprintf(output, "%s%s[%u]=%lg\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), arrayindex, val);
        else
            fprintf(output, "%s%s=%lg\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), val);
    }
};

template<bool printarraydim>
struct TypePrinter<bool, printarraydim> {
    static void print(const bool& val, const std::string& fieldname, FILE* output, int tabcount, size_t arrayindex) {
        if (printarraydim)
            fprintf(output, "%s%s[%u]=%s\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), arrayindex, val ? "true" : "false");
        else
            fprintf(output, "%s%s=%s\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), val ? "true" : "false");
    }
};

template<bool printarraydim>
struct TypePrinter<std::string, printarraydim> {
    static void print(const std::string& val, const std::string& fieldname, FILE* output, int tabcount, size_t arrayindex) {
        if (printarraydim)
            fprintf(output, "%s%s[%u]=%s\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), arrayindex, val.c_str());
        else
            fprintf(output, "%s%s=%s\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), val.c_str());
    }
};

template<bool printarraydim>
struct TypePrinter<google::protobuf::EnumValueDescriptor, printarraydim> {
    static void print(const google::protobuf::EnumValueDescriptor& val, const std::string& fieldname, FILE* output, int tabcount, size_t arrayindex) {
        if (printarraydim)
            fprintf(output, "%s%s[%u]=%s\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), arrayindex, val.name().c_str());
        else
            fprintf(output, "%s%s=%s\n", std::string(tabcount,' ').c_str(), fieldname.c_str(), val.name().c_str());
    }
};

template<bool printarraydim>
struct TypePrinter<google::protobuf::Message, printarraydim> {
    static void print(const google::protobuf::Message& val, const std::string& fieldname, FILE* output, int tabcount, size_t arrayindex) {
        if (printarraydim) {
            fprintf(output, "%s%s[%u]=", std::string(tabcount,' ').c_str(), fieldname.c_str(), arrayindex);
            parseMessageToFILE(val, output, tabcount);
        } else {
            fprintf(output, "%s%s=", std::string(tabcount,' ').c_str(), fieldname.c_str());
            parseMessageToFILE(val, output, tabcount);
        }
    }
};

int main (int argc, char **argv) {
    Options options;
    if (!getOptions(argc, argv, &options)) {
        print_usage();
    }

    FILE* output = nullptr;
    FILE* error = nullptr;
    Pkg package;

    if (!options.outputFile_.empty()) {
        output = fopen(options.outputFile_.c_str(), "wt");
        error = output;
    } else {
        output = stdout;
        error = stderr;
    }

    for (const auto pkg : options.inputFiles_) {
        if (!loadPkgFromFile(pkg.c_str(), &package)) {
            fprintf(error, "Failed to open package %s\n", pkg.c_str());
            continue;
        }
        fprintf(output, "Open package %s\n", pkg.c_str());
        if (options.listResources_) {
            parseMessageToFILE(package.header_, output);
        }

        auto header_size = package.headerSize_;
        auto* base_ptr = package.data_.data()+header_size;
        for (int i=0, n=package.header_.entries_size(); i<n; ++i) {
            auto& entry = package.header_.entries(i);
            google::protobuf::io::ArrayInputStream resourcefilestream(base_ptr+entry.entryoffset(), (int)entry.entrysize());
            google::protobuf::io::CodedInputStream resourcestream(&resourcefilestream);

            if (options.dumpReadable_) {
                Heart::proto::MessageContainer data_container;
                data_container.ParseFromCodedStream(&resourcestream);
                auto* msg = types::createMessageFromName(data_container.type_name().c_str());
                if (msg) {
                    msg->ParseFromString(data_container.messagedata());
                    parseMessageToFILE(*msg, output);
                    delete msg;
                }
            }
        }
    }

    if (output != stdout) {
        fclose(output);
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////
// It's a real shame this has to be done here.
// For the future is would probably work splitting types_database.h/.cpp into
// a dynamic library. Then another dynamic library with these lines of code in it
// could be compiled separately and pkginspector could link to that at startup
// (say by always loading types.(so|dll) and dynamically reflect unknown message types.
//////////////////////////////////////////////////////////////////////////
//register types
// DEFINE_AND_REGISTER_TYPE(Heart::proto, ServiceRegister);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, ServiceUnregister);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, ConsoleLogUpdate);
// //package.proto
// DEFINE_AND_REGISTER_TYPE(Heart::proto, PackageEntry);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, PackageHeader);
// //resource_common.proto
// DEFINE_AND_REGISTER_TYPE(Heart::proto, MessageContainer);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, Colour);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, ResourceSection);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, ResourceHeader);
// //resource_material_fx.proto
// DEFINE_AND_REGISTER_TYPE(Heart::proto, SamplerState);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, BlendState);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, DepthStencilState);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, RasterizerState);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, MaterialPass);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, MaterialTechnique);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, MaterialGroup);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, MaterialParameter);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, MaterialResource);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, MaterialSampler);
// //resource_mesh.proto
// DEFINE_AND_REGISTER_TYPE(Heart::proto, VertexStream);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, Renderable);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, Mesh);
// //resource_shader.proto
// DEFINE_AND_REGISTER_TYPE(Heart::proto, ShaderIncludeSource);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, ShaderInputStream);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, ShaderResource);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, ShaderResourceContainer);
// //resource_texture.proto
// DEFINE_AND_REGISTER_TYPE(Heart::proto, TextureMip);
// DEFINE_AND_REGISTER_TYPE(Heart::proto, TextureResource);
// //resource_font.proto
// DEFINE_AND_REGISTER_TYPE(Heart::proto, TTFResource);
