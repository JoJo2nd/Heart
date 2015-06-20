/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "minfs.h"
#include "resource_common.pb.h"
#include "resource_font.pb.h"
#include "builder.pb.h"
#include "getopt.h"
#include <fstream>
#include <memory>
#include <iostream>

#if defined (_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable:4244)
#   pragma warning(disable:4267)
#else
#   pragma error ("Unknown platform")
#endif
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/io/coded_stream.h"

#if defined (_MSC_VER)
#   pragma warning(pop)
#endif

#ifdef _WIN32
#   include <io.h>
#   include <fcntl.h>
#endif

static const char argopts[] = "vi:";
static struct option long_options[] = {
    { "version", no_argument, 0, 'z' },
    { 0, 0, 0, 0 }
};

#define fatal_error_check(x, msg, ...) if (!(x)) {fprintf(stderr, msg, __VA_ARGS__); exit(-1);}
#define fatal_error(msg, ...) fatal_error_check(false, msg, __VA_ARGS__)

int main(int argc, char* argv[]) {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#endif
    using namespace Heart;
    google::protobuf::io::IstreamInputStream input_stream(&std::cin);
    Heart::builder::Input input_pb;

    int c;
    int option_index = 0;
    bool verbose = false, use_stdin = true;

    while ((c = gop_getopt_long(argc, argv, argopts, long_options, &option_index)) != -1) {
        switch (c) {
        case 'z': fprintf(stdout, "heart font builder v0.8.0"); exit(0);
        case 'v': verbose = 1; break;
        case 'i': {
            std::ifstream input_file_stream;
            input_file_stream.open(optarg, std::ios_base::binary | std::ios_base::in);
            if (input_file_stream.is_open()) {
                google::protobuf::io::IstreamInputStream file_stream(&input_file_stream);
                input_pb.ParseFromZeroCopyStream(&file_stream);
                use_stdin = false;
            }
        } break;
        default: return 2;
        }
    }

    if (use_stdin) {
        input_pb.ParseFromZeroCopyStream(&input_stream);
    }
    
    std::vector<char> scratchbuffer;
    scratchbuffer.reserve(1024*10);
    const char* ttf_path=input_pb.resourceinputpath().c_str();
    size_t filesize=minfs_get_file_size(ttf_path);
    scratchbuffer.resize(filesize+1);
    FILE* f = fopen(ttf_path, "rt");
    fatal_error_check(f, "Unable to open output file %s", ttf_path);
    fread(scratchbuffer.data(), 1, filesize, f);
    fclose(f);
    scratchbuffer[filesize] = 0;

    proto::TTFResource resource_container;
    resource_container.set_ttfdata(scratchbuffer.data(), filesize);

    Heart::builder::Output output;
    output.add_filedependency(input_pb.resourceinputpath());
    //write the resource header
    output.mutable_pkgdata()->set_type_name(resource_container.GetTypeName());
    output.mutable_pkgdata()->set_messagedata(resource_container.SerializeAsString());

    google::protobuf::io::OstreamOutputStream filestream(&std::cout);
    return output.SerializeToZeroCopyStream(&filestream) ? 0 : -2;
}

