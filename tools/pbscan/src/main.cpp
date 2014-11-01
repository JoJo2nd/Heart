/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "getopt.h"
#if defined (_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable:4244)
#   pragma warning(disable:4267)
#endif
#include "google/protobuf/compiler/importer.h"
#if defined (_MSC_VER)
#   pragma warning(pop)
#endif
#include <vector>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <algorithm>


const char options[] = {"lrao:"};
static struct option long_options[] = {
    {"output",          required_argument,  0, 'o'},
    {0, 0, 0, 0}
};
enum class Option { // ensure these match options above
    optOutputToFile,
};

static void print_usage() {
    printf("pkginspector: ");
    printf(
        "Usage: pbscan [options] filepath1 filepath2 ... filepath2\n"
        "Available options are:\n"
        "\toutput                 - output to file.\n"
    );
}

struct Options {
    std::vector<std::string> inputFiles_;
    std::string              outputFile_;
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
       case 'o': opt = Option::optOutputToFile; break;
       case '?':
           /* getopt_long already printed an error message. */
       default:
           return false;
       }

       switch (opt)
       {
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

class ErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector {
public:
    ErrorCollector() 
        : hasError(0) {}

    void AddError(const std::string & filename, int line, int column, const std::string & message) override {
        std::stringstream s(errors);
        hasError = -1;
        s << filename << "(" << (line+1) << " ): error :" << message << "\n";
    }

    int        hasError;
    std::string errors;
};

struct PBType {
    std::string name;
    std::string pbnamespace;
    std::string include;
};

const char hdrStr[] = 
" \
/******************************************************************** \n\
    Auto Generated File - Do not edit \n\
*********************************************************************/ \n\
/******************************************************************** \n\
    Written by James Moran \n\
    Please see the file HEART_LICENSE.txt in the source's root directory. \n\
*********************************************************************/ \n\
#pragma once \n\
 \n\
namespace google { \n\
namespace protobuf { \n\
    class Message; \n\
    class Descriptor; \n\
    class Reflection; \n\
} \n\
} \n\
 \n\
namespace types { \n\
    typedef google::protobuf::Message*            (*TypeConstructProc)(); \n\
    typedef const google::protobuf::Descriptor*   (*TypeDecriptorProc)(); \n\
    typedef const google::protobuf::Reflection*   (*TypeReflectProc)(); \n\
 \n\
    void                                    initialiseTypes(); \n\
    google::protobuf::Message*              createMessageFromName(const char* ); \n\
    const google::protobuf::Descriptor*     getDescriptorFromName(const char* ); \n\
    const google::protobuf::Reflection*     getReflectionFromName(const char* ); \n\
    template < typename t_ty > \n\
    const t_ty* getDefaultTypeObject() { \n\
        return &t_ty::default_instance(); \n\
    } \n\
} \n\
 \n\
";

const char srcStrHdr1[] = 
"\
/******************************************************************** \n\
    Auto Generated File - Do not edit \n\
*********************************************************************/ \n\
/******************************************************************** \n\
    Written by James Moran \n\
    Please see the file HEART_LICENSE.txt in the source's root directory. \n\
*********************************************************************/ \n\
 \n\
#include \"type_database.h\" \n\
#include <string> \n\
#include <unordered_map> \n\
#if defined (_MSC_VER)\n\
#   pragma warning(push)\n\
#   pragma warning(disable:4244)\n\
#   pragma warning(disable:4267)\n\
#endif\n";

const char srcStrHdr2[] = 
"\n\
#if defined (_MSC_VER)\n\
#   pragma warning(pop)\n\
#endif\n\
 \n\
namespace types \n\
{ \n\
namespace \n\
{ \n\
    struct Type \n\
    { \n\
        TypeConstructProc   construct_; \n\
        TypeDecriptorProc   decriptor_; \n\
        TypeReflectProc     reflection_; \n\
    }; \n\
 \n\
    struct TypeDatabase  \n\
    { \n\
        typedef std::unordered_map< std::string, Type > TypeTable; \n\
 \n\
        TypeTable database_; \n\
 \n\
        static TypeDatabase* get() { \n\
            static TypeDatabase instance_; \n\
            return &instance_; \n\
        } \n\
 \n\
        bool getType(const char* name, Type* out_type_info) { \n\
            auto found_itr = database_.find(name); \n\
            if (found_itr == database_.end()) { \n\
                return false; \n\
            } \n\
            *out_type_info = found_itr->second; \n\
            return true; \n\
        } \n\
 \n\
        void addType(const char* name, Type in_type) { \n\
            database_.emplace(name, in_type); \n\
        } \n\
    }; \n\
} \n\
 \n\
    google::protobuf::Message* createMessageFromName(const char* name) { \n\
        auto* db = TypeDatabase::get(); \n\
        Type type_info; \n\
        if (!db->getType(name, &type_info)) { \n\
            return nullptr; \n\
        } \n\
        return type_info.construct_(); \n\
    } \n\
 \n\
    const google::protobuf::Descriptor* getDescriptorFromName(const char* name) { \n\
        auto* db = TypeDatabase::get(); \n\
        Type type_info; \n\
        if (!db->getType(name, &type_info)) { \n\
            return nullptr; \n\
        } \n\
        return type_info.decriptor_(); \n\
    } \n\
 \n\
    const google::protobuf::Reflection* getReflectionFromName(const char* name) { \n\
        auto* db = TypeDatabase::get(); \n\
        Type type_info; \n\
        if (!db->getType(name, &type_info)) { \n\
            return nullptr; \n\
        } \n\
        return type_info.reflection_(); \n\
    } \n\
 \n\
    static bool registerType(const char* name, TypeConstructProc construct_proc, TypeDecriptorProc descriptor_proc, TypeReflectProc reflector_proc) { \n\
        auto* db = TypeDatabase::get(); \n\
        Type type_info = {construct_proc, descriptor_proc, reflector_proc}; \n\
        db->addType(name, type_info); \n\
        return true; \n\
    } \n\
 \n\
} \n\
#define DEFINE_AND_REGISTER_TYPE(name_space, type_name) \\\n\
    struct type_##type_name { \\\n\
        static google::protobuf::Message* typeCreate() { \\\n\
            return getDefaultTypeObject()->New(); \\\n\
        } \\\n\
        static const google::protobuf::Descriptor* typeDescriptor() { \\\n\
            return name_space::type_name::descriptor(); \\\n\
        } \\\n\
        static const google::protobuf::Reflection* typeReflection() { \\\n\
            return getDefaultTypeObject()->GetReflection(); \\\n\
        } \\\n\
        static const name_space::type_name* getDefaultTypeObject() { \\\n\
            return &name_space::type_name::default_instance(); \\\n\
        }\\\n\
    };  \\\n\
    registerType(type_##type_name::getDefaultTypeObject()->GetTypeName().c_str(), type_##type_name::typeCreate, type_##type_name::typeDescriptor, type_##type_name::typeReflection); \n\
 \n\
namespace types { \n\
    void initialiseTypes() { \n\
";
const char srcStrFtr[] = 
" \n\
    }\n\
} \n\
";

int main(int argc, char **argv) {
    Options options;
    if (!getOptions(argc, argv, &options)) {
        print_usage();
    }
    using google::protobuf::compiler::DiskSourceTree;
    using google::protobuf::compiler::Importer;
    
    typedef std::map<std::string, PBType>::value_type PBTypePair;
    std::map<std::string, PBType> typemap;
    std::set<std::string> includes;
    ErrorCollector errorcollector;
    DiskSourceTree sourcetree;

    FILE* error = stderr;

    sourcetree.MapPath("", "./");

    for (const auto& pbf : options.inputFiles_) {
        Importer imp(&sourcetree, &errorcollector);
        auto localpbf = pbf;
        std::replace(localpbf.begin(), localpbf.end(), '\\', '/');
        auto rootpbf = localpbf.substr(0, localpbf.find_last_of('/'));
        if (!rootpbf.empty()) {
            localpbf = localpbf.substr(localpbf.find_last_of('/')+1);
            sourcetree.MapPath("", rootpbf.c_str());
        }
        printf("Scaning %s\n", pbf.c_str());
        auto* fd = imp.Import(localpbf);
        if (!fd) {
            break;
        }
        for (auto i=0, n=fd->message_type_count(); i<n; ++i) {
            if (typemap.find(fd->message_type(i)->full_name()) == typemap.end()) {
                auto pbt = fd->message_type(i);
                PBType t;
                t.name = pbt->name();
                t.pbnamespace = pbt->full_name();
                t.pbnamespace.replace(t.pbnamespace.find(t.name), std::string::npos, "");
                if (!t.pbnamespace.empty()) {
                    // there will be a trailing '.' so remove it
                    t.pbnamespace.pop_back();
                }
                for (auto p = t.pbnamespace.find("."); p != std::string::npos; p = t.pbnamespace.find(".")) {
                    t.pbnamespace.erase(p, 1);
                    t.pbnamespace.insert(p, "::");
                }
                
                t.include = pbt->file()->name();
                t.include.replace(t.include.find("."), std::string::npos, ".pb.h");
                typemap.insert(PBTypePair(fd->message_type(i)->full_name(), t));
                if (includes.find(t.include) == includes.end()) {
                    includes.insert(t.include);
                }
            }

        }
    }

    if (!errorcollector.hasError) {
        FILE* hdroutput = fopen("type_database.h", "wb");
        FILE* cppoutput = fopen("type_database.cpp", "wb");
        fwrite(hdrStr, 1, sizeof(hdrStr)-1, hdroutput);
        fclose(hdroutput);

        fwrite(srcStrHdr1, 1, sizeof(srcStrHdr1)-1, cppoutput);
        for (const auto& i : includes) {
            fprintf(cppoutput, "#include \"%s\"\n", i.c_str());
        }
        fwrite(srcStrHdr2, 1, sizeof(srcStrHdr2)-1, cppoutput);
        for (const auto& i : typemap) {
            fprintf(cppoutput, "\t\tDEFINE_AND_REGISTER_TYPE(%s, %s);\n", i.second.pbnamespace.c_str(), i.second.name.c_str());
        }
        fwrite(srcStrFtr, 1, sizeof(srcStrFtr)-1, cppoutput);
        fclose(cppoutput);
    } else {
        printf("%s", errorcollector.errors.c_str());
    }
    return errorcollector.hasError;
}