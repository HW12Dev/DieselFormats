#pragma once

/// 
/// Functions and classes to read and parse compiled Diesel Script (.dxe) files.
/// 

#include "fileio/reader.h"
#include "diesel/shared.h"

namespace diesel {
  namespace graw {
    namespace dieselscriptinternals { // not 1:1, it is a close enough approximation
      struct ScriptedFunction {
      public:
        ScriptedFunction(const std::string& name, uint32_t functionOffset) : name(name), functionOffset(functionOffset) {}

        std::string name;
        uint32_t functionOffset;
      };
      class ScriptedClass {
      public:
        ScriptedClass();
      public:
        std::string class_name;
        std::vector<ScriptedFunction> functions;
        std::vector<std::string> variables;
      };
      class ClassFunction {};
    }
    class DieselScript {
    public:
      DieselScript();

    public:
      void ReadCompiledDXE(Reader& reader, diesel::DieselFormatsLoadingParameters version);

    private:
      std::vector<dieselscriptinternals::ScriptedClass*> classes;
    };
  }
}