#include "diesel/graw/dieselscript.h"

#include <cassert>

#include <vector>
#include <string>

#include <iostream>

// dsl::script::Parser::ImCall::generate_body: 45

#define DIESELSCRIPT_OPCODES \
DIESELSCRIPT_OPCODE(ImConstNumber, 6) \
DIESELSCRIPT_OPCODE(ImConstString, 7) \
\
DIESELSCRIPT_OPCODE(ImConstBoolean_0, 8) \
DIESELSCRIPT_OPCODE(ImConstBoolean_1, 9) \
\
DIESELSCRIPT_OPCODE(ImCommand_0, 47) \
DIESELSCRIPT_OPCODE(ImCommand_1, 21) \
DIESELSCRIPT_OPCODE(ImCommand_3, 46) \
DIESELSCRIPT_OPCODE(ImCommand_4, 40) \
DIESELSCRIPT_OPCODE(ImCommand_5, 0) \
DIESELSCRIPT_OPCODE(ImCommand_6, 10) \
DIESELSCRIPT_OPCODE(ImCommand_7, 11) \
DIESELSCRIPT_OPCODE(ImCommand_8, 18) \
DIESELSCRIPT_OPCODE(ImCommand_9, 19) \
DIESELSCRIPT_OPCODE(ImCommand_10, 20) \
\
DIESELSCRIPT_OPCODE(ImBinOp_0, 22) \
DIESELSCRIPT_OPCODE(ImBinOp_1, 23) \
DIESELSCRIPT_OPCODE(ImBinOp_2, 24) \
DIESELSCRIPT_OPCODE(ImBinOp_3, 25) \
DIESELSCRIPT_OPCODE(ImBinOp_4, 26) \
DIESELSCRIPT_OPCODE(ImBinOp_5, 33) \
DIESELSCRIPT_OPCODE(ImBinOp_6, 34) \
DIESELSCRIPT_OPCODE(ImBinOp_7, 35) \
DIESELSCRIPT_OPCODE(ImBinOp_8, 36) \
DIESELSCRIPT_OPCODE(ImBinOp_9, 37) \
DIESELSCRIPT_OPCODE(ImBinOp_10, 38) \
\
DIESELSCRIPT_OPCODE(ImUnOp, 27) \
\
DIESELSCRIPT_OPCODE(ImResizeLocal, 39) \
DIESELSCRIPT_OPCODE(ImGotoCond_1, 41) \
DIESELSCRIPT_OPCODE(ImGotoCond_2, 42) \
DIESELSCRIPT_OPCODE(ImGotoCond_3, 43) \
DIESELSCRIPT_OPCODE(ImMember_44, 44) \
DIESELSCRIPT_OPCODE(ImCall, 45)

#define DIESELSCRIPT_OPCODE(name, opcode_byte) name = opcode_byte,

enum DieselScriptOpcodes {
  DIESELSCRIPT_OPCODES
};

#undef DIESELSCRIPT_OPCODE

#define DIESELSCRIPT_OPCODE(name, opcode_byte) if(opcode == opcode_byte) return #name;

const char* GetNameOfOpcode(DieselScriptOpcodes opcode) {
  DIESELSCRIPT_OPCODES;

  return "UnknownOpcode";
}

#undef DIESELSCRIPT_OPCODE

using namespace diesel::graw::dieselscriptinternals;

namespace diesel {
  namespace graw {
    namespace dieselscriptinternals {
      void ParseOpcodes(Reader& reader, diesel::DieselFormatsLoadingParameters version);
    }

    DieselScript::DieselScript() {
    }

    void DieselScript::ReadCompiledDXE(Reader& reader, diesel::DieselFormatsLoadingParameters version) {
      auto tag = reader.ReadType<uint32_t>(); // "DXE\x00"
      assert(tag == _byteswap_ulong('DXE\x00'));

      //auto size = reader.ReadType<uint32_t>();

      //reader.AddPosition(8); // remaining 12 bytes are whatever was in the stack
      //char* data = new char[size];
      //reader.ReadBytesToBuffer(data, size);

      auto fileSize = reader.ReadType<uint32_t>(); // file size, excluding the 16 byte header

      reader.ReadType<uint32_t>();
      reader.ReadType<uint32_t>();

      std::vector<std::string> imports;

      ScriptedClass* current_class = nullptr;// this[30]

      while (true) {
        auto operation = reader.ReadType<uint8_t>();

        if (operation == 0) {
          break; // Can't break out of a loop in a switch statement
        }

        switch (operation) {
        case 1: // import a Diesel Script object
        {
          imports.push_back(reader.ReadString());
        }
        break;
        case 2:
        {
          auto declare = reader.ReadString();
          if (current_class) {
            current_class->variables.push_back(declare);
          }
          else {
            __debugbreak();
          }

        }
        break;
        case 3:
        {
          auto v32 = reader.ReadType<uint32_t>();
          auto v33 = reader.ReadType<uint32_t>();

          if (v33 == 0) {
            // TODO: implement
          }

          reader.AddPosition(-4);

          auto unk1 = reader.ReadString();

          if (current_class) {
            current_class->functions.push_back(ScriptedFunction(unk1, v32));
          }
          else {
            __debugbreak();
          }


          //class_functions.push_back(unk1);
        }
        break;

        case 4: // start class context
        {
          auto unk2 = reader.ReadString();
          current_class = new ScriptedClass();
          current_class->class_name = unk2;
          this->classes.push_back(current_class);
        }
        break;

        case 5: // end class context
        {
          current_class = nullptr;
        }
        break;

        default:
          __debugbreak();
          break;
        }
      }

      for (auto clazz : this->classes) {
        for (auto& function : clazz->functions) {
          reader.SetPosition(function.functionOffset + 0x10);

          std::cout << function.name << ":\n";
          ParseOpcodes(reader, version);

        }
      }

      __debugbreak();
    }
  }
}

diesel::graw::dieselscriptinternals::ScriptedClass::ScriptedClass()
{
}

void diesel::graw::dieselscriptinternals::ParseOpcodes(Reader& reader, diesel::DieselFormatsLoadingParameters version) {
  bool shouldReadOpcodes = true;
  while (shouldReadOpcodes) {
    auto opcode = (DieselScriptOpcodes)reader.ReadType<uint8_t>();

    std::cout << GetNameOfOpcode(opcode);

    if (opcode == ImCommand_5) {
      std::cout << "\n";
      break;
    }

    switch (opcode) {
    case DieselScriptOpcodes::ImResizeLocal:
    {
      auto localVariable = reader.ReadType<uint8_t>();
      std::cout << ", " << (int)localVariable;
    }
    break;
    case DieselScriptOpcodes::ImConstNumber:
    {
      auto number = reader.ReadType<float>();
      std::cout << ", " << number;
    }
    break;
    case DieselScriptOpcodes::ImConstString:
    {
      auto string = reader.ReadString();
      std::cout << ", " << string;
    }
    break;

    case DieselScriptOpcodes::ImGotoCond_1:
    {
      reader.AddPosition(4);
    }
    break;

    case DieselScriptOpcodes::ImCommand_0:
    case DieselScriptOpcodes::ImCommand_1:
    case DieselScriptOpcodes::ImCommand_3:
    case DieselScriptOpcodes::ImCommand_4:
    case DieselScriptOpcodes::ImCommand_5:
    case DieselScriptOpcodes::ImCommand_6:
    case DieselScriptOpcodes::ImCommand_7:
    case DieselScriptOpcodes::ImCommand_8:
    case DieselScriptOpcodes::ImCommand_9:
    case DieselScriptOpcodes::ImCommand_10:
      break;
    case DieselScriptOpcodes::ImBinOp_0:
    case DieselScriptOpcodes::ImBinOp_1:
    case DieselScriptOpcodes::ImBinOp_2:
    case DieselScriptOpcodes::ImBinOp_3:
    case DieselScriptOpcodes::ImBinOp_4:
    case DieselScriptOpcodes::ImBinOp_5:
    case DieselScriptOpcodes::ImBinOp_6:
    case DieselScriptOpcodes::ImBinOp_7:
    case DieselScriptOpcodes::ImBinOp_8:
    case DieselScriptOpcodes::ImBinOp_9:
    case DieselScriptOpcodes::ImBinOp_10:
      break;

    case DieselScriptOpcodes::ImMember_44:
    {
      auto member = reader.ReadString();
      std::cout << ", " << member << std::endl;
    }
    break;

    case DieselScriptOpcodes::ImCall:
    {
      auto parameters = reader.ReadType<uint8_t>();
      std::cout << ", params=" << (int)parameters;
    }
    break;

    default:
      __debugbreak();
    }

    std::cout << "\n";
  }
}