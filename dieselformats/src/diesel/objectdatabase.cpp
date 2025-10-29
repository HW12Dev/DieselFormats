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

      for (diesel::objectdatabase::typeidclasses::PersistentObject* object : _object_list) {
        object->post_load();
      }
    }

    bool ObjectDatabase::Write(Writer& writer, const diesel::DieselFormatsLoadingParameters& loadParameters) {

      writer.WriteType<uint32_t>(-1);

      size_t fileSizePosition = writer.GetPosition();
      writer.WriteType<uint32_t>(0);

      writer.WriteType<uint32_t>((uint32_t)this->_object_list.size());

      SavingReferenceMap ref_map;

      for (unsigned int i = 0; i < (unsigned int)this->_object_list.size(); i++) {
        diesel::objectdatabase::typeidclasses::PersistentObject* obj = this->_object_list[i];

        writer.WriteType<TypeId>(obj->type_id());

        ref_map.WriteRef(writer, obj);
        ref_map.AddRef(obj, i + 1);

        size_t writeObjectSizePosition = writer.GetPosition();
        writer.WriteType<uint32_t>(0);

        obj->save(writer, ref_map, loadParameters);

        size_t objectSize = writer.GetPosition() - writeObjectSizePosition - 4;

        size_t nextObjectStartPos = writer.GetPosition();

        writer.SetPosition(writeObjectSizePosition);
        writer.WriteType<uint32_t>((uint32_t)objectSize);
        writer.SetPosition(nextObjectStartPos);

      }

      size_t fileSize = writer.GetPosition();
      writer.SetPosition(fileSizePosition);
      writer.WriteType<uint32_t>((uint32_t)fileSize);

      ref_map.WriteReferences(writer);

      writer.SetPosition(fileSize);

      return true;
    }


    ObjectDatabase::~ObjectDatabase() {
      for (auto object : this->_object_list) {
        delete object;
      }
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

      if (typeId == typeids::SkinBones)
        return new SkinBones();

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
    void SavingReferenceMap::WriteReferences(Writer& writer)
    {
      for (auto& reference : referencesToWrite) {
        if (!this->objectRefids.contains(reference.second))
          continue;
        writer.SetPosition(reference.first);
        writer.WriteType<RefId>(this->objectRefids[reference.second]);
      }
    }
}
}

#pragma region Persistent Object Classes

diesel::objectdatabase::typeidclasses::PersistentObject::PersistentObject() {
  _name = diesel::modern::Idstring(-1);
}

diesel::objectdatabase::typeidclasses::PersistentObject::~PersistentObject() {
}

void diesel::objectdatabase::typeidclasses::PersistentObject::load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) {
  if (diesel::DoLoadParametersHaveIdstrings(loadParameters)) {
    this->_name = reader.ReadType<uint64_t>();
  }
  else {
    // It is more convienient to load old names that are string literals as hashes and add them to the hashlist
    std::string str = reader.ReadString();
    diesel::modern::GetGlobalHashlist()->AddSourceToHashlist(str);
    this->_name = diesel::modern::Idstring(str);
  }
}

void diesel::objectdatabase::typeidclasses::PersistentObject::save(Writer& writer, SavingReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters)
{
  if (_name == diesel::modern::Idstring(-1ull)) // if name wasn't read
    return;

  if (diesel::DoLoadParametersHaveIdstrings(loadParameters)) {
    writer.WriteType<uint64_t>(this->_name);
  }
  else {
    writer.WriteString(diesel::modern::GetGlobalHashlist()->GetIdstringSource(this->_name));
  }
}

void diesel::objectdatabase::typeidclasses::PersistentObject::post_load() {}

#pragma endregion
