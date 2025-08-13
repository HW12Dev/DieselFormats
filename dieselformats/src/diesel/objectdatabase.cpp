#include "diesel/objectdatabase.h"

#include "diesel/objectdatabase_model.h"
#include "diesel/objectdatabase_shaders.h"

#include "diesel/modern/hashlist.h"

#include <cassert>

/// REMOVE
#include <iostream>
#include <set>


namespace diesel {
  namespace objectdatabase {
    ObjectDatabase::ObjectDatabase(Reader& reader2, const diesel::DieselFormatsLoadingParameters& loadParameters) {
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

      for (int i = 0; i < count; i++) {
        auto type_id = reader.ReadType<TypeId>();
        auto ref_id = reader.ReadType<RefId>();

        auto objectSize = reader.ReadType<uint32_t>();

        auto objectStartPos = reader.GetPosition();

        diesel::objectdatabase::typeidclasses::PersistentObject* obj = diesel::objectdatabase::typeidclasses::ConstructPersistentObjectFromTypeId(type_id, loadParameters.version);

        obj->load(reader, ref_map, loadParameters);

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

    std::vector<typeidclasses::PersistentObject*>& ObjectDatabase::GetObjects() {
      return this->_object_list;
    }

#define REGISTER_TYPEID(clazz, clazzTypeId)
    //if(typeId == clazzTypeId) return new clazz(version);
    typeidclasses::PersistentObject* typeidclasses::ConstructPersistentObjectFromTypeId(TypeId typeId, diesel::EngineVersion version) {

      if (typeId == typeids::PersistentObject)
        return new PersistentObject();

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

      if (typeId == typeids::D3DShaderLibraryData)
        return new D3DShaderLibraryData();
      if (typeId == typeids::D3DShaderData)
        return new D3DShaderData();
      if (typeId == typeids::D3DShaderPassData)
        return new D3DShaderPassData();

      if (typeId == typeids::GCMShaderLibrary)
        return new GCMShaderLibrary();
      if (typeId == typeids::GCMShader)
        return new GCMShader();
      if (typeId == typeids::GCMShaderPass)
        return new GCMShaderPass();

      static std::set<TypeId> reportedTypeIds;

      if (reportedTypeIds.find(typeId) == reportedTypeIds.end()) {
        std::cout << "Unregistered type id: " << typeId << " (0x" << diesel::modern::hex((const char*)&typeId, sizeof(typeId)) << ")" << " : " << TypeIdToStr(typeId) << std::endl;
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
}

#pragma region Persistent Object Classes

diesel::objectdatabase::typeidclasses::PersistentObject::PersistentObject() {
}

diesel::objectdatabase::typeidclasses::PersistentObject::~PersistentObject() {
}

void diesel::objectdatabase::typeidclasses::PersistentObject::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) {
  auto isIdstring = loadParameters.version > diesel::EngineVersion::MODERN_VERSION_START;
  if (isIdstring) {
    this->_name = reader.ReadType<uint64_t>();
  }
  else {
    // It is more convienient to load old names that are string literals as hashes and add them to the hashlist
    std::string str = reader.ReadString();
    diesel::modern::GetGlobalHashlist()->AddSourceToHashlist(str);
    this->_name = diesel::modern::Idstring(str);
  }
}


#pragma endregion
