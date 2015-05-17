
#pragma once

#include <string>
#include <map>
#include <google/protobuf/compiler/code_generator.h>

namespace google {
namespace protobuf {
class EnumDescriptor;
namespace io {
    class Printer;
}
namespace compiler {

class CppDiffGenerator : public CodeGenerator {
 public:
  CppDiffGenerator();
  virtual ~CppDiffGenerator();

  virtual bool Generate(const FileDescriptor* file,
                        const string& parameter,
                        GeneratorContext* context,
                        string* error) const;

 private:

     static string GetFullClassName(const string& class_name);
     static vector<string> GetPackageNamespaces(const string& package);
     static string GetPackageNamespacePrefix(const string& package);
     static string GetOutputFileName(const string& generator_name, const FileDescriptor* file);
     static string GetOutputFileName(const string& generator_name, const string& file);
};

}  // namespace compiler
}  // namespace protobuf

}  // namespace google
