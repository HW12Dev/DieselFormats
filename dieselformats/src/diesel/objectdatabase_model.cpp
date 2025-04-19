#include "diesel/objectdatabase_model.h"

#include "diesel/modern/hashlist.h"

#include <cassert>


unsigned int diesel::objectdatabase::typeidclasses::model::channel_size[diesel::objectdatabase::typeidclasses::model::ChannelType::CT_COUNT] = {
  0,
  4,
  8,
  0x0C,
  0x10,
  4,
  4,
  8,
  0xC,
};

using namespace diesel::objectdatabase::typeidclasses::model;

void diesel::objectdatabase::typeidclasses::Model::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) { // dsl::Model::load from PAYDAY 2 Linux/PDTH v1/Lead and Gold/Ballistics decomp/RAID decomp
  Object3D::load(reader, ref_map, loadParameters);

  auto type = reader.ReadType<uint32_t>();

  if (type == 6) { // present in pd2 linux + raid
    // TODO: extra information is stored when this condition is true
    return;
  }

  auto geometryproducer_refid = reader.ReadType<RefId>();
  auto indexproducer_refid = reader.ReadType<RefId>();

  auto atom_count = reader.ReadType<uint32_t>();

  for (int i = 0; i < atom_count; i++) { // TODO: More changes for ballistics support
    auto& atom = this->_atoms.emplace_back(IndexedRenderAtom());

    if (loadParameters.version == EngineVersion::LEAD_AND_GOLD) {
      atom.layer = reader.ReadType<uint32_t>();
    }
    else {
      atom.layer = -1;
    }
    atom.vertex_offset = reader.ReadType<uint32_t>();
    atom.primitives = reader.ReadType<uint32_t>();
    atom.index_offset = reader.ReadType<uint32_t>();
    atom.vertices = reader.ReadType<uint32_t>();


    if (geometryproducer_refid) {
      auto& ref_value = atom.streams.emplace_back(nullptr);
      ref_map.load_ref(geometryproducer_refid, &ref_value);
    }
    if (indexproducer_refid) {
      ref_map.load_ref(indexproducer_refid, &atom.indices);
    }
  }

  reader.ReadType<RefId>(); // MaterialGroup refid


  auto lightset_refid = reader.ReadType<RefId>();
  ref_map.load_ref(lightset_refid, &this->_light_set);


  this->_properties = reader.ReadType<uint32_t>();
  this->_bounding_volume = reader.ReadType<BoundingVolume>();

  if(loadParameters.version != EngineVersion::BALLISTICS) // not present in ballistics (no support for skeletal meshes)
    reader.ReadType<RefId>(); // bones refid
}


void diesel::objectdatabase::typeidclasses::AuthorTag::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters)
{
  PersistentObject::load(reader, ref_map, loadParameters);

  this->_author_tag = reader.ReadString();
  this->_last_export_source = reader.ReadString();
  this->_version = reader.ReadType<uint32_t>();
}


void diesel::objectdatabase::typeidclasses::Animatable::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters)
{
  PersistentObject::load(reader, ref_map, loadParameters);

  // _param_block
  auto i = reader.ReadType<uint32_t>();
  for (int j = 0; j < i; j++) {
    reader.ReadType<uint32_t>();
  }

  reader.AddPosition(8);
}

void diesel::objectdatabase::typeidclasses::Object3D::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters)
{
  Animatable::load(reader, ref_map, loadParameters);

  if (loadParameters.version == EngineVersion::BALLISTICS)
    reader.AddPosition(sizeof(Matrix4)); // Ballistics has another matrix read to this + 88

  reader.ReadBytesToBuffer(&this->_local_tm, sizeof(this->_local_tm));
  reader.ReadBytesToBuffer(&this->_local_tm.t, sizeof(this->_local_tm.t));

  auto parent_refid = reader.ReadType<RefId>();
  ref_map.load_ref(parent_refid, &this->_parent);
}

void diesel::objectdatabase::typeidclasses::Material::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters)
{
  PersistentObject::load(reader, ref_map, loadParameters);

  reader.ReadType<uint32_t>(); // dummy_int
  reader.ReadType<uint32_t>(); // dummy_int
  reader.AddPosition(0x10); // dummy_colour
  reader.AddPosition(0x10); // dummy_colour
  reader.ReadType<float>(); // dummy_float
  reader.ReadType<float>(); // dummy_float
  static_assert((sizeof(uint32_t) * 2 + 0x10 * 2 + sizeof(float) * 2) == 48);


  auto num_textures = reader.ReadType<uint32_t>();

  // TODO: continue
}

void diesel::objectdatabase::typeidclasses::MaterialGroup::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters)
{
  auto material_count = reader.ReadType<uint32_t>();

  for (int i = 0; i < material_count; i++) {
    auto refid = reader.ReadType<RefId>();

    auto& ref_value = this->_materials.emplace_back(nullptr);

    ref_map.load_ref(refid, &ref_value);
  }
}


void diesel::objectdatabase::typeidclasses::AnimationData::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters)
{
  PersistentObject::load(reader, ref_map, loadParameters);

  this->start_time = reader.ReadType<float>();
  this->end_time = reader.ReadType<float>();

  auto animatable_count = reader.ReadType<uint32_t>();
  for (int i = 0; i < animatable_count; i++) {
    auto animatable_refid = reader.ReadType<RefId>();

    auto& ref_value = this->_animatable_list.emplace_back(nullptr);

    ref_map.load_ref(animatable_refid, &ref_value);
  }
}


diesel::objectdatabase::typeidclasses::Geometry::~Geometry()
{
  delete[] this->_vertices;
}

void diesel::objectdatabase::typeidclasses::Geometry::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters)
{
  auto _size = reader.ReadType<uint32_t>();

  auto channel_count = reader.ReadType<uint32_t>();

  int offset = 0;
  for (int i = 0; i < channel_count; i++) {
    auto type = reader.ReadType<ChannelType>();
    auto component = reader.ReadType<VertexComponent>();

    this->_format.push_back(GeometryProducerChannelDesc{ .type = type, .component = component });

    this->_channel_offset.push_back(offset);

    assert(type != ChannelType::CT_UNINITIALIZED);

    this->_vertex_size += channel_size[type];

    offset += channel_size[type];
  }

  this->_vertices = new char[this->_vertex_size * _size];

  reader.ReadBytesToBuffer(this->_vertices, this->_vertex_size * _size);

  PersistentObject::load(reader, ref_map, loadParameters); // name can be not present (check if the buffer has ended)
}

void diesel::objectdatabase::typeidclasses::Topology::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters)
{
  this->_type = reader.ReadType<IndexProducer::Type>();

  auto _num_indices = reader.ReadType<uint32_t>();

  for (int i = 0; i < _num_indices; i++) {
    this->_indices.push_back(reader.ReadType<uint16_t>());
  }

  auto num_groupings = reader.ReadType<uint32_t>();

  for (int i = 0; i < num_groupings; i++) {
    this->_groupings.push_back(reader.ReadType<uint8_t>());
  }

  PersistentObject::load(reader, ref_map, loadParameters); // name can be not present (check if the buffer has ended)
}

void diesel::objectdatabase::typeidclasses::TopologyIP::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters)
{
  auto topology_refid = reader.ReadType<RefId>();

  ref_map.load_ref(topology_refid, &this->_topology);
}

void diesel::objectdatabase::typeidclasses::PassThroughGP::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters)
{
  auto geometry_refid = reader.ReadType<RefId>();
  ref_map.load_ref(geometry_refid, &this->_geometry);
  auto topology_refid = reader.ReadType<RefId>();
  ref_map.load_ref(topology_refid, &this->_topology);
}

