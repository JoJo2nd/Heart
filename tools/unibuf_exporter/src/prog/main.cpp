/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "getopt.h"
#include "rapidjson/rapidjson.h"
#include "lib_uniform_buffer.h"
#include "minfs.h"
#include <vector>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <algorithm>

static const size_t FS_MAX_PATH = 260;

struct Options {
    bool success = false;
    bool cxx_output = false;
    bool hlsl_output = false;
    bool verbose = false;
    std::string destDirectory;
    std::vector<std::string> inputFiles;
};

Options getOptions(int argc, char **argv) {
    Options opts;
    const char argopts[] = "vxho:";
    static struct option long_options[] = {
        { "verbose", no_argument, 0, (int)'v' },
        { "cxx", no_argument, 0, (int)'x' },
        { "hlsl", no_argument, 0, (int)'h' },
        { "output_dir", required_argument, 0, (int)'o'},
        { 0, 0, 0, 0 }
    };
    int c;
    int option_index;
    while ((c = gop_getopt_long(argc, argv, argopts, long_options, &option_index)) != -1) {
        switch (c) {
        case 'v': opts.verbose = 1; break;
        case 'o': opts.destDirectory = optarg; break;
        case 'x': opts.cxx_output = true; break;
        case 'h': opts.hlsl_output = true; break;
        default: break;
        }
    }

    // For any remaining command line arguments (not options).
    while (optind < argc) {
        opts.inputFiles.push_back(argv[optind++]);
    }

    opts.success = true;
    return opts;
}

std::string exportToCXX(const ub::Descriptor& ubDesc, const std::string& srcPath);
std::string exportToHLSL(const ub::Descriptor& ubDesc, const std::string& srcPath);

int main(int argc, char **argv) {
    Options options = getOptions(argc, argv);
    if (!options.success) {
        return -1;
    }
    
    std::string full_dest_path;
    for (const auto& input_file : options.inputFiles) {
        char dest_path[FS_MAX_PATH];
        char filename[FS_MAX_PATH];
        char tmp_path[FS_MAX_PATH];
        minfs_path_without_ext(input_file.c_str(), tmp_path, FS_MAX_PATH);
        minfs_path_leaf(tmp_path, filename, FS_MAX_PATH);
        minfs_path_join(options.destDirectory.c_str(), filename, tmp_path, FS_MAX_PATH);

        size_t file_size = minfs_get_file_size(input_file.c_str());
        std::vector<char> file_data;
        file_data.resize(file_size, 0);
        FILE* f = fopen(input_file.c_str(), "rb");
        if (!f) {
            fprintf(stderr, "Failed to open input file %s", input_file.c_str());
            exit(1);
        }
        fread(file_data.data(), 1, file_size, f);
        fclose(f);
        ub::Descriptor ubDesc;
        if (!ubDesc.loadFromJSON(file_data.data(), file_data.size())) {
            fprintf(stderr, "Failed to parse JSON uniform buffer description.");
            exit(2);
        }

        if (options.cxx_output) {
            full_dest_path = tmp_path;
            full_dest_path += ".hpp";
            std::string cxx_data = exportToCXX(ubDesc, input_file);
            FILE* f = fopen(full_dest_path.c_str(), "wb");
            if (!f) {
                fprintf(stderr, "Failed to open output file '%s' for writing", dest_path);
                exit(3);
            }
            fwrite(cxx_data.data(), 1, cxx_data.size(), f);
            fclose(f);
        }
        if (options.hlsl_output) {
            full_dest_path = tmp_path;
            full_dest_path += ".hlsl.h";
            std::string hlsl_data = exportToHLSL(ubDesc, input_file);
            FILE* f = fopen(full_dest_path.c_str(), "wb");
            if (!f) {
                fprintf(stderr, "Failed to open output file '%s' for writing", dest_path);
                exit(3);
            }
            fwrite(hlsl_data.data(), 1, hlsl_data.size(), f);
            fclose(f);
        }
    }
    return 0;
}

std::string exportToCXX(const ub::Descriptor& ubDesc, const std::string& srcPath ) {
    std::stringstream cxx;
    cxx << "/********************************************************************\n"
    "\tAuto generated file. Don't edit here, edit '" << srcPath << "' instead.\n"
    "********************************************************************/\n";
    cxx << "#pragma once\n";
    cxx << "#include \"render/hShaderTypes.h\"\n";
    cxx << "#include \"render/hProgramReflectionInfo.h\"\n";
    cxx << "\n\nstruct " << ubDesc.getName() << " {\n";

    for (size_t i=0,n=ubDesc.getElementCount(); i<n; ++i) {
        const ub::Element& element = ubDesc.getElement(i);
        switch (element.type) {
        case ub::DataType::Float1: cxx << "\tfloat " << element.name << ";\n"; break;
        case ub::DataType::Float2: cxx << "\tfloat2 " << element.name << ";\n"; break;
        case ub::DataType::Float3: cxx << "\tfloat3 " << element.name << ";\n"; break;
        case ub::DataType::Float4: cxx << "\tfloat4 " << element.name << ";\n"; break;
        case ub::DataType::Float22: cxx << "\tfloat22 " << element.name << ";\n"; break;
        case ub::DataType::Float23: cxx << "\tfloat23 " << element.name << ";\n"; break;
        case ub::DataType::Float24: cxx << "\tfloat24 " << element.name << ";\n"; break;
        case ub::DataType::Float32: cxx << "\tfloat32 " << element.name << ";\n"; break;
        case ub::DataType::Float33: cxx << "\tfloat33 " << element.name << ";\n"; break;
        case ub::DataType::Float34: cxx << "\tfloat34 " << element.name << ";\n"; break;
        case ub::DataType::Float42: cxx << "\tfloat42 " << element.name << ";\n"; break;
        case ub::DataType::Float43: cxx << "\tfloat43 " << element.name << ";\n"; break;
        case ub::DataType::Float44: cxx << "\tfloat44 " << element.name << ";\n"; break;
        case ub::DataType::Int1: cxx << "\tint1 " << element.name << ";\n"; break;
        case ub::DataType::Int2: cxx << "\tint2 " << element.name << ";\n"; break;
        case ub::DataType::Int3: cxx << "\tint3 " << element.name << ";\n"; break;
        case ub::DataType::Int4: cxx << "\tint4 " << element.name << ";\n"; break;
        case ub::DataType::Uint1: cxx << "\tuint1 " << element.name << ";\n"; break;
        case ub::DataType::Uint2: cxx << "\tuint2 " << element.name << ";\n"; break;
        case ub::DataType::Uint3: cxx << "\tuint3 " << element.name << ";\n"; break;
        case ub::DataType::Uint4: cxx << "\tuint4 " << element.name << ";\n"; break;
        }
    }

    cxx << "inline static const Heart::hRenderer::hUniformLayoutDesc* getLayout(hUint32* count = nullptr) {\n";
    cxx << "\tusing namespace Heart::hRenderer;\n\tstatic hUniformLayoutDesc layout[" << ubDesc.getElementCount() << "] = {\n";
    uint32_t currentByteOffset = 0;
    for (size_t i = 0, n = ubDesc.getElementCount(); i < n; ++i) {
        const ub::Element& element = ubDesc.getElement(i);
        switch (element.type) {
        case ub::DataType::Float1: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Float, " << currentByteOffset << " },\n"; currentByteOffset += 16; break;
        case ub::DataType::Float2: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Float2, " << currentByteOffset << " },\n"; currentByteOffset += 16; break;
        case ub::DataType::Float3: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Float3, " << currentByteOffset << " },\n"; currentByteOffset += 16; break;
        case ub::DataType::Float4: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Float4, " << currentByteOffset << " },\n"; currentByteOffset += 16; break;
        case ub::DataType::Float22: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Float22, " << currentByteOffset << " },\n"; currentByteOffset += 32; break;
        case ub::DataType::Float23: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Float23, " << currentByteOffset << " },\n"; currentByteOffset += 32; break;
        case ub::DataType::Float24: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Float24, " << currentByteOffset << " },\n"; currentByteOffset += 32; break;
        case ub::DataType::Float32: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Float32, " << currentByteOffset << " },\n"; currentByteOffset += 48; break;
        case ub::DataType::Float33: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Float33, " << currentByteOffset << " },\n"; currentByteOffset += 48; break;
        case ub::DataType::Float34: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Float34, " << currentByteOffset << " },\n"; currentByteOffset += 48; break;
        case ub::DataType::Float42: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Float42, " << currentByteOffset << " },\n"; currentByteOffset += 64; break;
        case ub::DataType::Float43: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Float43, " << currentByteOffset << " },\n"; currentByteOffset += 64; break;
        case ub::DataType::Float44: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Float44, " << currentByteOffset << " },\n"; currentByteOffset += 64; break;
        case ub::DataType::Int1: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Int, " << currentByteOffset << " },\n"; currentByteOffset += 16; break;
        case ub::DataType::Int2: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Int2, " << currentByteOffset << " },\n"; currentByteOffset += 16; break;
        case ub::DataType::Int3: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Int3, " << currentByteOffset << " },\n"; currentByteOffset += 16; break;
        case ub::DataType::Int4: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Int4, " << currentByteOffset << " },\n"; currentByteOffset += 16; break;
        case ub::DataType::Uint1: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Uint, " << currentByteOffset << " },\n"; currentByteOffset += 16; break;
        case ub::DataType::Uint2: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Uint2, " << currentByteOffset << " },\n"; currentByteOffset += 16; break;
        case ub::DataType::Uint3: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Uint3, " << currentByteOffset << " },\n"; currentByteOffset += 16; break;
        case ub::DataType::Uint4: cxx << "\t\t{ \"" << element.name << "\", ShaderParamType::Uint4, " << currentByteOffset << " },\n"; currentByteOffset += 16; break;
        }
    }
    cxx << "\t};\n\tif (count) *count = getLayoutCount();\n\treturn layout;\n}\n";
    cxx << "inline static hUint32 getLayoutCount() {\n";
    cxx << "\treturn " << ubDesc.getElementCount() << ";\n";
    cxx << "};\n\n";
    cxx << "};\n\n";
    return cxx.str();
}

std::string exportToHLSL(const ub::Descriptor& ubDesc, const std::string& srcPath) {
    std::stringstream hlsl;
    hlsl << "/********************************************************************\n"
        "\tAuto generated file. Don't edit here, edit '" << srcPath << "' instead.\n"
        "********************************************************************/\n";
    hlsl << "#ifndef __AUTO_" << ubDesc.getName() << "_HEADER_GUARD__\n";
    hlsl << "#define __AUTO_" << ubDesc.getName() << "_HEADER_GUARD__\n";
    hlsl << "\n\ncbuffer " << ubDesc.getName() << " {\n";

    for (size_t i = 0, n = ubDesc.getElementCount(); i<n; ++i) {
        const ub::Element& element = ubDesc.getElement(i);
        switch (element.type) {
        case ub::DataType::Float1: hlsl << "\tfloat " << element.name << ";\n"; break;
        case ub::DataType::Float2: hlsl << "\tfloat2 " << element.name << ";\n"; break;
        case ub::DataType::Float3: hlsl << "\tfloat3 " << element.name << ";\n"; break;
        case ub::DataType::Float4: hlsl << "\tfloat4 " << element.name << ";\n"; break;
        case ub::DataType::Float22: hlsl << "\tfloat2x2 " << element.name << ";\n"; break;
        case ub::DataType::Float23: hlsl << "\tfloat2x3 " << element.name << ";\n"; break;
        case ub::DataType::Float24: hlsl << "\tfloat2x4 " << element.name << ";\n"; break;
        case ub::DataType::Float32: hlsl << "\tfloat3x2 " << element.name << ";\n"; break;
        case ub::DataType::Float33: hlsl << "\tfloat3x3 " << element.name << ";\n"; break;
        case ub::DataType::Float34: hlsl << "\tfloat3x4 " << element.name << ";\n"; break;
        case ub::DataType::Float42: hlsl << "\tfloat4x2 " << element.name << ";\n"; break;
        case ub::DataType::Float43: hlsl << "\tfloat4x3 " << element.name << ";\n"; break;
        case ub::DataType::Float44: hlsl << "\tfloat4x4 " << element.name << ";\n"; break;
        case ub::DataType::Int1: hlsl << "\tint " << element.name << ";\n"; break;
        case ub::DataType::Int2: hlsl << "\tint2 " << element.name << ";\n"; break;
        case ub::DataType::Int3: hlsl << "\tint3 " << element.name << ";\n"; break;
        case ub::DataType::Int4: hlsl << "\tint4 " << element.name << ";\n"; break;
        case ub::DataType::Uint1: hlsl << "\tuint " << element.name << ";\n"; break;
        case ub::DataType::Uint2: hlsl << "\tuint2 " << element.name << ";\n"; break;
        case ub::DataType::Uint3: hlsl << "\tuint3 " << element.name << ";\n"; break;
        case ub::DataType::Uint4: hlsl << "\tuint4 " << element.name << ";\n"; break;
        }
    }
    hlsl << "};\n\n";
    hlsl << "#endif // __AUTO_" << ubDesc.getName() << "_HEADER_GUARD__\n";

    return hlsl.str();
}
