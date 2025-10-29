#pragma once

#include "diesel/objectdatabase.h"

namespace diesel {
  namespace objectdatabase {
    namespace typeidclasses {
      class D3DShaderData;
      class D3DShaderPassData;
      class D3DShaderLibraryData;

      namespace shaders {
        namespace d3d {
          enum class D3DStateVariableType : uint8_t {
            CONSTANT = 0x0,
            REFERENCE = 0x1,
            UNINITIALIZED = 0x2
          };

          class D3DStateVariable { // dsl::wd3d::StateVariable
          public:
            D3DStateVariable();

            static D3DStateVariable Read(Reader& reader, const diesel::DieselFormatsLoadingParameters& loadParameters);
            void Write(Writer& writer, const diesel::DieselFormatsLoadingParameters& loadParameters) const;

          public:
            D3DStateVariableType type;
            unsigned int constant;
            diesel::modern::Idstring reference;
          };
          class D3DStateBlock { // dsl::wd3d::D3DStateBlock
          public:
            D3DStateBlock();
            ~D3DStateBlock();

          public:
            std::vector<std::pair<int, D3DStateVariable>> states;

            std::string sampler_block_name_ogl; // not from diesel, but linux OpenGL shaders contain the sampler name as unhashed test. this will be an empty string if this isn't from an OpenGL shader or if it's not a sampler state block
          };
        }
      }

      class D3DShaderPassData : public PersistentObject {
      public:
        D3DShaderPassData();
        ~D3DShaderPassData();
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        virtual void save(Writer& writer, SavingReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(D3DShaderPassData);

        void SetVertexShader(const char* source, size_t sourceSize);
        void SetPixelShader(const char* source, size_t sourceSize);

      private:
      public: // REMOVE
        shaders::d3d::D3DStateBlock _render_states;
        std::vector<std::pair<int, shaders::d3d::D3DStateBlock>> _dx9_sampler_state_blocks; // stores any sampler state blocks for DirectX 9 and OpenGL
        std::vector<std::pair<diesel::modern::Idstring, shaders::d3d::D3DStateBlock>> _dx11_sampler_state_blocks; // stores any sampler state blocks for DirectX 10 & 11


        char* _compiled_vertex_shader; // diesel stores these as a vector of unsigned char, here they are stored as char* with their size in another variable
        unsigned long long _compiled_vertex_shader_size;
        char* _compiled_pixel_shader;
        unsigned long long _compiled_pixel_shader_size;
      };

      class D3DShaderData : public PersistentObject {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        virtual void save(Writer& writer, SavingReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(D3DShaderData);

      private:
      public: // REMOVE
        std::vector<std::pair<diesel::modern::Idstring, D3DShaderPassData*>> _layers;
        D3DShaderLibraryData* shaderLibrary;
      };

      class D3DShaderLibraryData : public PersistentObject {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        virtual void save(Writer& writer, SavingReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(D3DShaderLibraryData);

      private:
      public: // REMOVE
        std::map<diesel::modern::Idstring, D3DShaderData*> _shaders;
      };

      class GCMShaderPass : public PersistentObject {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(GCMShaderPass);
      };
      class GCMShaderLibrary : public PersistentObject {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(GCMShaderLibrary);
      };
      class GCMShader : public PersistentObject {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(GCMShader);
      };
    }
  }
}