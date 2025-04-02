#include "diesel/objectdatabase.h"

#include "diesel/modern/hashlist.h"

#include <cassert>

/// REMOVE
#include <iostream>
#include <set>

unsigned int diesel::typeidclasses::channel_size[diesel::typeidclasses::ChannelType::CT_COUNT] = {
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

namespace diesel {
  ObjectDatabase::ObjectDatabase(Reader& reader2, diesel::EngineVersion version) {
    auto readBufSize = reader2.GetFileSize();
    char* readBuf = new char[readBufSize];
    reader2.ReadBytesToBuffer(readBuf, readBufSize);
    Reader reader(readBuf, readBufSize);
    ReferenceMap ref_map = ReferenceMap();

    auto count = reader.ReadType<int32_t>();

    int32_t size = 0;
    if (count == -1) {
      size = reader.ReadType<int32_t>();
      count = reader.ReadType<int32_t>();
    }

    this->_object_list.reserve(count);

    for (int i = 0; i < count; i++) {
      auto type_id = reader.ReadType<TypeId>();
      auto ref_id = reader.ReadType<RefId>();

      auto objectSize = reader.ReadType<uint32_t>();

      auto objectStartPos = reader.GetPosition();

      diesel::typeidclasses::PersistentObject* obj = diesel::typeidclasses::ConstructPersistentObjectFromTypeId(type_id, version);

      obj->load(reader, ref_map, version);

      ref_map.AddRef(ref_id, obj);

      this->_object_list.push_back(obj);

      reader.SetPosition(objectStartPos + objectSize);
    }

    ref_map.AssignAllReferences();
  }



  ObjectDatabase::~ObjectDatabase() {
    for (auto object : this->_object_list) {
      delete object;
    }
  }

#define REGISTER_TYPEID(clazz, clazzTypeId)
//if(typeId == clazzTypeId) return new clazz(version);
  typeidclasses::PersistentObject* typeidclasses::ConstructPersistentObjectFromTypeId(TypeId typeId, diesel::EngineVersion version) {

    if (typeId == typeids::Object3D)
      return new Object3D();

    if (typeId == typeids::AuthorTag)
      return new AuthorTag();

    if (typeId == typeids::Model)
      return new Model();

    if (typeId == typeids::Material)
      return new Material();

    if (typeId == typeids::MaterialGroup)
      return new MaterialGroup();

    if (typeId == typeids::Topology)
      return new Topology();

    if (typeId == typeids::Geometry)
      return new Geometry();

    if (typeId == typeids::Animatable)
      return new Animatable();

    if (typeId == typeids::AnimationData)
      return new AnimationData();

    if (typeId == typeids::TopologyIP)
      return new TopologyIP();
    if (typeId == typeids::PassThroughGP)
      return new PassThroughGP();

    static std::set<TypeId> reportedTypeIds;

    if (reportedTypeIds.find(typeId) == reportedTypeIds.end()) {
      std::cout << "Unregistered type id: " << typeId << " : " << TypeIdToStr(typeId) << std::endl;
      reportedTypeIds.insert(typeId);
    }

    return new PersistentObject();
  }
#undef REGISTER_TYPEID

  void ReferenceMap::AddRef(RefId refId, typeidclasses::PersistentObject* obj) {
    this->refidToObjMap.insert({ refId, obj });
  }
  typeidclasses::PersistentObject* ReferenceMap::GetObjectFromRefId(RefId refId) {
    auto find = this->refidToObjMap.find(refId);

    if (find != this->refidToObjMap.end())
      return find->second;

    return nullptr;
  }
  void ReferenceMap::AssignAllReferences() {
    for (auto& ref : this->referencesToFulfill) {
      *ref.second = this->GetObjectFromRefId(ref.first);
    }
    this->referencesToFulfill.clear();
  }

#define TYPE_ID_ENTRY(clazz, clazzTypeId) if(typeId == clazzTypeId) return #clazz;

  const char* TypeIdToStr(TypeId typeId) {
    TYPE_ID_LIST
    return "Unknown TypeId";
  }

#undef TYPE_ID_ENTRY
}

#pragma region Persistent Object Classes

diesel::typeidclasses::PersistentObject::PersistentObject() {
#ifndef NDEBUG
  this->_name_str = "not populated";
#endif
}

diesel::typeidclasses::PersistentObject::~PersistentObject() {
}

void diesel::typeidclasses::PersistentObject::load(Reader& reader, ReferenceMap& ref_map, diesel::EngineVersion version) {
  auto isIdstring = version > diesel::EngineVersion::MODERN_VERSION_START;
  if (isIdstring) {
    this->_name = reader.ReadType<uint64_t>();
  }
  else {
    // It is more convienient to load old names that are string literals as hashes and add them to the hashlist
    std::string str = reader.ReadString();
    diesel::modern::GetGlobalHashlist()->AddSourceToHashlist(str);
    this->_name = diesel::modern::Idstring(str);
  }

#ifndef NDEBUG
  diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(this->_name, this->_name_str);
#endif
}

void diesel::typeidclasses::AuthorTag::load(Reader& reader, ReferenceMap& ref_map, diesel::EngineVersion version) {
  PersistentObject::load(reader, ref_map, version);

  this->_author_tag = reader.ReadString();
  this->_last_export_source = reader.ReadString();
  this->_version = reader.ReadType<uint32_t>();
}


void diesel::typeidclasses::Animatable::load(Reader& reader, ReferenceMap& ref_map, diesel::EngineVersion version) {
  PersistentObject::load(reader, ref_map, version);

  // _param_block
  auto i = reader.ReadType<uint32_t>();
  for (int j = 0; j < i; j++) {
    reader.ReadType<uint32_t>();
  }

  reader.AddPosition(8);
}

void diesel::typeidclasses::Object3D::load(Reader& reader, ReferenceMap& ref_map, diesel::EngineVersion version) {
  Animatable::load(reader, ref_map, version);
  
  if(version == EngineVersion::BALLISTICS)
    reader.AddPosition(sizeof(Matrix4)); // Ballistics has another matrix read to this + 88

  reader.ReadBytesToBuffer(&this->_local_tm, sizeof(this->_local_tm));
  reader.ReadBytesToBuffer(&this->_local_tm.t, sizeof(this->_local_tm.t));

  auto parent_refid = reader.ReadType<RefId>();
  ref_map.load_ref(parent_refid, &this->_parent);
}

void diesel::typeidclasses::Material::load(Reader& reader, ReferenceMap& ref_map, diesel::EngineVersion version) {
  PersistentObject::load(reader, ref_map, version);

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

void diesel::typeidclasses::MaterialGroup::load(Reader& reader, ReferenceMap& ref_map, diesel::EngineVersion version) {
  auto material_count = reader.ReadType<uint32_t>();

  this->_materials.reserve(material_count);

  for (int i = 0; i < material_count; i++) {
    auto refid = reader.ReadType<RefId>();

    auto& ref_value = this->_materials.emplace_back(nullptr);

    ref_map.load_ref(refid, &ref_value);
  }
}


void diesel::typeidclasses::AnimationData::load(Reader& reader, ReferenceMap& ref_map, diesel::EngineVersion version) {
  PersistentObject::load(reader, ref_map, version);

  this->start_time = reader.ReadType<float>();
  this->end_time = reader.ReadType<float>();

  auto animatable_count = reader.ReadType<uint32_t>();
  for (int i = 0; i < animatable_count; i++) {
    auto animatable_refid = reader.ReadType<RefId>();

    auto& ref_value = this->_animatable_list.emplace_back(nullptr);

    ref_map.load_ref(animatable_refid, &ref_value);
  }
}


diesel::typeidclasses::Geometry::~Geometry() {
  delete[] this->_vertices;
}

void diesel::typeidclasses::Geometry::load(Reader& reader, ReferenceMap& ref_map, diesel::EngineVersion version) {
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

  PersistentObject::load(reader, ref_map, version); // name can be not present (check if the buffer has ended)
}

void diesel::typeidclasses::Topology::load(Reader& reader, ReferenceMap& ref_map, diesel::EngineVersion version) {
  this->_type = reader.ReadType<IndexProducer::Type>();

  auto _num_indices = reader.ReadType<uint32_t>();

  this->_indices.reserve(_num_indices);

  for (int i = 0; i < _num_indices; i++) {
    this->_indices.push_back(reader.ReadType<uint16_t>());
  }

  auto num_groupings = reader.ReadType<uint32_t>();

  for (int i = 0; i < num_groupings; i++) {
    this->_groupings.push_back(reader.ReadType<uint8_t>());
  }

  PersistentObject::load(reader, ref_map, version); // name can be not present (check if the buffer has ended)
}

void diesel::typeidclasses::TopologyIP::load(Reader& reader, ReferenceMap& ref_map, diesel::EngineVersion version) {
  auto topology_refid = reader.ReadType<RefId>();

  ref_map.load_ref(topology_refid, &this->_topology);
}

void diesel::typeidclasses::PassThroughGP::load(Reader& reader, ReferenceMap& ref_map, diesel::EngineVersion version) {
  auto geometry_refid = reader.ReadType<RefId>();
  ref_map.load_ref(geometry_refid, &this->_geometry);
  auto topology_refid = reader.ReadType<RefId>();
  ref_map.load_ref(topology_refid, &this->_topology);
}

#pragma endregion
