#include "diesel/objectdatabase.h"

#include "diesel/modern/hashlist.h"

/// REMOVE
#include <iostream>
#include <set>

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
      count = reader.ReadType<int32_t>();
      size = reader.ReadType<int32_t>();
    }

    this->_object_list.reserve(size);

    for (int i = 0; i < size; i++) {
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

  reader.ReadBytesToBuffer(&this->_local_tm, sizeof(this->_local_tm));
  reader.ReadBytesToBuffer(&this->_local_tm.t, sizeof(this->_local_tm.t));

  auto parent_refid = reader.ReadType<RefId>();
  ref_map.load_ref(parent_refid, &this->_parent);
}

void diesel::typeidclasses::Model::load(Reader& reader, ReferenceMap& ref_map, diesel::EngineVersion version) { // dsl::Model::load from PAYDAY 2 Linux
  Object3D::load(reader, ref_map, version);

  auto unk1 = reader.ReadType<uint32_t>();

  if (unk1 == 6) {
    return;
  }

  auto geometryProducerRefId = reader.ReadType<RefId>();
  auto indexProducerRefId = reader.ReadType<RefId>();
}
