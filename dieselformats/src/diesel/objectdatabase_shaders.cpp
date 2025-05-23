#include "diesel/objectdatabase_shaders.h"

#include "diesel/modern/hashlist.h"


using namespace diesel::objectdatabase::typeidclasses::shaders::d3d;

static_assert(sizeof(D3DStateVariableType) == 0x1);

diesel::modern::Idstring ReadStringOrIdstringFromLoadParameters(Reader& reader, const diesel::DieselFormatsLoadingParameters& loadParameters) {
  diesel::modern::Idstring name;
  if (!diesel::DoLoadParametersHaveIdstrings(loadParameters)) {
    auto str = reader.ReadString();
    name = diesel::modern::Idstring(str);
    diesel::modern::GetGlobalHashlist()->AddSourceToHashlist(str);
  }
  else {
    name = reader.ReadType<uint64_t>();
  }
  return name;
}

D3DStateVariable::D3DStateVariable() {
  this->type = D3DStateVariableType::UNINITIALIZED;
  this->constant = -1;
  this->reference = diesel::modern::Idstring(-1);
}

D3DStateBlock::D3DStateBlock() {
  this->states = std::vector<std::pair<int, D3DStateVariable>>();
  this->sampler_block_name_ogl = "";
}

D3DStateBlock::~D3DStateBlock() {
}

diesel::objectdatabase::typeidclasses::D3DShaderPassData::D3DShaderPassData() {
  this->_compiled_pixel_shader = nullptr;
  this->_compiled_pixel_shader_size = -1;
  this->_compiled_vertex_shader = nullptr;
  this->_compiled_vertex_shader_size = -1;

  this->_dx9_sampler_state_blocks = std::vector<std::pair<int, D3DStateBlock>>();
  this->_dx11_sampler_state_blocks = std::vector<std::pair<diesel::modern::Idstring, D3DStateBlock>>();
}

diesel::objectdatabase::typeidclasses::D3DShaderPassData::~D3DShaderPassData() {
  if (this->_compiled_pixel_shader)
    delete[] this->_compiled_pixel_shader;
  if (this->_compiled_vertex_shader)
    delete[] this->_compiled_vertex_shader;
}

/// 
/// Tested with:
///  - Terminator Salvation (Xbox 360)
/// 
void diesel::objectdatabase::typeidclasses::D3DShaderLibraryData::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) {
  if (loadParameters.renderer == Renderer::UNSPECIFIED) // If no renderer has been specified, skip loading shader information. otherwise a lot of incorrect data will be read and an infinite loop will likely be entered.
    return;
  if (loadParameters.renderer == Renderer::DIRECTX8) // Loading shaders from an Object Database with DirectX 8 should not be possible, shaders for this DirectX version were written in Nvidia's ARB assembly language, compiled with NVASM and saved to fragments.nvo.
    return;

  auto shaders = reader.ReadType<uint32_t>();

  for (int i = 0; i < shaders; i++) {
    diesel::modern::Idstring name = ReadStringOrIdstringFromLoadParameters(reader, loadParameters);

    auto refid = reader.ReadType<RefId>(); // ref id for attached D3DShaderData

    auto val = this->_shaders.insert({ name, (D3DShaderData*)nullptr }).first;
    ref_map.load_ref(refid, &val->second);
  }
}

void diesel::objectdatabase::typeidclasses::D3DShaderData::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) {
  if (loadParameters.renderer == Renderer::UNSPECIFIED) // If no renderer has been specified, skip loading shader information. otherwise a lot of incorrect data will be read and an infinite loop will likely be entered.
    return;
  if (loadParameters.renderer == Renderer::DIRECTX8) // Loading shaders from an Object Database with DirectX 8 should not be possible, shaders for this DirectX version were written in Nvidia's ARB assembly language, compiled with NVASM and saved to fragments.nvo.
    return;

  auto layerCount = reader.ReadType<uint32_t>();

  for(int i = 0; i < layerCount; i++) {
    diesel::modern::Idstring name = ReadStringOrIdstringFromLoadParameters(reader, loadParameters);

    auto shaderLibraryDataRefId = reader.ReadType<RefId>();

    auto shaderPassDataRefId = reader.ReadType<RefId>();

    auto& val = this->_layers.emplace_back(name, (D3DShaderPassData*)nullptr);
    ref_map.load_ref(shaderPassDataRefId, &val.second);
  }
}

void diesel::objectdatabase::typeidclasses::D3DShaderPassData::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) {
  if (loadParameters.renderer == Renderer::UNSPECIFIED) // If no renderer has been specified, skip loading shader information. otherwise a lot of incorrect data will be read and an infinite loop will likely be entered.
    return;
  if (loadParameters.renderer == Renderer::DIRECTX8) // Loading shaders from an Object Database with DirectX 8 should not be possible, shaders for this DirectX version were written in Nvidia's ARB assembly language, compiled with NVASM and saved to fragments.nvo.
    return;

  auto renderStateCount = reader.ReadType<uint32_t>();

  for (int i = 0; i < renderStateCount; i++) {
    auto state_first = reader.ReadType<uint32_t>();
    
    D3DStateVariable state = D3DStateVariable();

    state.type = reader.ReadType<D3DStateVariableType>(); // state.second.type

    if (state.type != D3DStateVariableType::CONSTANT)
      state.reference = reader.ReadType<uint64_t>();
    else
      state.constant = reader.ReadType<uint32_t>();

    this->_render_states.states.emplace_back(std::make_pair(state_first, state));
  }

  auto samplerStateBlockCount = reader.ReadType<uint32_t>();

  for (int i = 0; i < samplerStateBlockCount; i++) {
    auto dx9_block_first = (int32_t)0;
    auto dx11_block_first = diesel::modern::Idstring();

    if (loadParameters.renderer == Renderer::DIRECTX11 || loadParameters.renderer == Renderer::DIRECTX10) {
      if (loadParameters.version <= EngineVersion::LEAD_AND_GOLD) {
        std::string str = reader.ReadString();
        dx11_block_first = diesel::modern::Idstring(str);
        diesel::modern::GetGlobalHashlist()->AddSourceToHashlist(str);
      }
      else {
        dx11_block_first = reader.ReadType<uint64_t>();
      }
    }
    else {
      dx9_block_first = reader.ReadType<int32_t>();
    }

    D3DStateBlock block = D3DStateBlock();

    if (loadParameters.renderer == Renderer::OPENGL) {
      block.sampler_block_name_ogl = reader.ReadString();
    }

    auto ns = reader.ReadType<uint32_t>();

    for (int j = 0; j < ns; j++) {
      auto state_first = reader.ReadType<uint32_t>();

      D3DStateVariable state = D3DStateVariable();

      state.type = (D3DStateVariableType)reader.ReadType<uint8_t>();

      if (state.type != D3DStateVariableType::CONSTANT)
        state.reference = reader.ReadType<uint64_t>();
      else
        state.constant = reader.ReadType<uint32_t>();

      block.states.emplace_back(std::make_pair(state_first, state));
    }

    if(loadParameters.renderer == Renderer::DIRECTX11 || loadParameters.renderer == Renderer::DIRECTX10)
      this->_dx11_sampler_state_blocks.emplace_back(std::make_pair(dx11_block_first, block));
    else
      this->_dx9_sampler_state_blocks.emplace_back(std::make_pair(dx9_block_first, block));
  }

  this->_compiled_vertex_shader_size = reader.ReadType<uint32_t>();
  this->_compiled_vertex_shader = new char[this->_compiled_vertex_shader_size];
  reader.ReadBytesToBuffer(this->_compiled_vertex_shader, this->_compiled_vertex_shader_size);
  this->_compiled_pixel_shader_size = reader.ReadType<uint32_t>();
  this->_compiled_pixel_shader = new char[this->_compiled_pixel_shader_size];
  reader.ReadBytesToBuffer(this->_compiled_pixel_shader, this->_compiled_pixel_shader_size);
}


///// PlayStation 3 (Cell) shaders


/// 
/// function address 0x004680f4 in PAYDAY: The Heist PS3 NPEA00331 1.00
/// 
void diesel::objectdatabase::typeidclasses::GCMShaderPass::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) { // 
  auto unk1 = reader.ReadType<uint32_t>();

  for (int i = 0; i < unk1; i++) {
    auto unk2 = reader.ReadType<uint32_t>();

    auto unk3 = reader.ReadType<uint8_t>();

    if (unk3) {
      auto unk4 = reader.ReadType<uint64_t>();
    }
    else {
      auto unk5 = reader.ReadType<uint32_t>();
    }
  }

  auto unk6 = reader.ReadType<uint32_t>();

  for (int i = 0; i < unk6; i++) {
    auto unk7 = reader.ReadType<uint32_t>();
    auto unk8 = reader.ReadType<uint32_t>();

    for (int j = 0; j < unk8; j++) {
      auto unk2 = reader.ReadType<uint32_t>();

      auto unk3 = reader.ReadType<uint8_t>();

      if (unk3) {
        auto unk4 = reader.ReadType<uint64_t>();
      }
      else {
        auto unk5 = reader.ReadType<uint32_t>();
      }
    }
  }

  auto vertex_size = reader.ReadType<uint32_t>();
  reader.AddPosition(vertex_size);
  auto fragment_size = reader.ReadType<uint32_t>();
  reader.AddPosition(fragment_size);
}

/// function address 0x004793ac in PAYDAY: The Heist PS3 NPEA00331 1.00
void diesel::objectdatabase::typeidclasses::GCMShader::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) {
  auto unk1 = reader.ReadType<uint32_t>();

  for (int i = 0; i < unk1; i++) {
    diesel::modern::Idstring name = ReadStringOrIdstringFromLoadParameters(reader, loadParameters);
    auto unk2 = reader.ReadType<uint32_t>();

    for (int i = 0; i < unk2; i++) {
      auto unk3 = reader.ReadType<uint32_t>();
    }
  }
}


void diesel::objectdatabase::typeidclasses::GCMShaderLibrary::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) {
  auto unk1 = reader.ReadType<uint32_t>();

  for (int i = 0; i < unk1; i++) {
    diesel::modern::Idstring name = ReadStringOrIdstringFromLoadParameters(reader, loadParameters);

    auto refid = reader.ReadType<RefId>(); // ref id for attached GCMShaderData

  }
  __debugbreak();
}

