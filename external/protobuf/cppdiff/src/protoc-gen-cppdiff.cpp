#include <string>
#include <stdlib.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/stubs/strutil.h>
#include "cpp_diff_code_generator.h"

int main(int argc, char* argv[]) {
#ifdef _MSC_VER
    _set_abort_behavior(0, ~0);
#endif  // !_MSC_VER

    google::protobuf::compiler::CppDiffGenerator generator;
    return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}