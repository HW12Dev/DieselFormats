#pragma once

#include "fileio/reader.h"
#include "diesel/modern/hash.h"
#include "diesel/shared.h"

#include <unordered_map>
#include <map>

// dsl::Diesel::Diesel
#define TYPE_ID_LIST \
TYPE_ID_ENTRY(PersistentObject, 0x2EB43C77) \
TYPE_ID_ENTRY(Object3D, 0xFFCD100) \
TYPE_ID_ENTRY(LightSet, 0x33552583) \
TYPE_ID_ENTRY(Model, 0x62212D88) \
TYPE_ID_ENTRY(AuthorTag, 0x7623C465) \
TYPE_ID_ENTRY(Geometry, 0x7AB072D3) \
TYPE_ID_ENTRY(SimpleTexture, 0x72B4D37) \
TYPE_ID_ENTRY(CubicTexture, 0x2C5D6201) \
TYPE_ID_ENTRY(VolumetricTexture, 0x1D0B1808) \
TYPE_ID_ENTRY(Material, 0x3C54609C) \
TYPE_ID_ENTRY(MaterialGroup, 0x29276B1D) \
TYPE_ID_ENTRY(NormalManagingGP, 0x2C1F096F) \
TYPE_ID_ENTRY(TextureSpaceGP, 0x5ED2532F) \
TYPE_ID_ENTRY(PassThroughGP, 0xE3A3B1CA) \
TYPE_ID_ENTRY(SkinBones, 0x65CC1825) \
TYPE_ID_ENTRY(Topology, 0x4C507A13) \
TYPE_ID_ENTRY(TopologyIP, 0x3B634BD) \
TYPE_ID_ENTRY(Camera, 0x46BF31A7) \
TYPE_ID_ENTRY(Light, 0xFFA13B80) \
\
TYPE_ID_ENTRY(ConstFloatController, 0x2060697E) \
TYPE_ID_ENTRY(StepFloatController, 0x6DA951B2) \
TYPE_ID_ENTRY(LinearFloatController, 0x76BF5B66) \
TYPE_ID_ENTRY(BezierFloatController, 0x29743550) \
\
TYPE_ID_ENTRY(ConstVector3Controller, 0x5B0168D0) \
TYPE_ID_ENTRY(StepVector3Controller, 0x544E238F) \
TYPE_ID_ENTRY(LinearVector3Controller, 0x26A5128C) \
TYPE_ID_ENTRY(BezierVector3Controller, 0x28DB639A) \
TYPE_ID_ENTRY(XYZVector3Controller, 0x33DA0FC4) \
\
TYPE_ID_ENTRY(ConstRotationController, 0x2E540F3C) \
TYPE_ID_ENTRY(EulerRotationController, 0x33606E8) \
TYPE_ID_ENTRY(QuatStepRotationController, 0x007FB371) \
TYPE_ID_ENTRY(QuatLinearRotationController, 0x648A206C) \
TYPE_ID_ENTRY(QuatBezRotationController, 0x197345A5) \
TYPE_ID_ENTRY(LookAtRotationController, 0x22126DC0) \
TYPE_ID_ENTRY(LookAtConstrRotationController, 0x679D695B) \
\
TYPE_ID_ENTRY(IKChainTarget, 0x3D756E0C) \
TYPE_ID_ENTRY(IKChainRotationController, 0xF6C1EEF7) \
\
TYPE_ID_ENTRY(CompositeVector3Controller, 0xDD41D329) \
TYPE_ID_ENTRY(CompositeRotationController, 0x95BB08F7) \
\
TYPE_ID_ENTRY(AnimationData, 0x5DC011B8) \
TYPE_ID_ENTRY(Animatable, 0x74F7363F) \
\
TYPE_ID_ENTRY(KeyEvents, 0x186A8BBF) \
\
TYPE_ID_ENTRY(D3DShaderLibraryData, 0x12812C1A) \
TYPE_ID_ENTRY(D3DShaderData, 0x7F3552D1) \
TYPE_ID_ENTRY(D3DShaderPassData, 0x214B1AAF) \
\
/* Sony Playstation 3 Shaders */ \
TYPE_ID_ENTRY(GCMShaderPass, 0x3C752FFA) \
TYPE_ID_ENTRY(GCMShader, 0x3DF69E08) \
TYPE_ID_ENTRY(GCMShaderLibrary, 0xF38A9F49)


#define PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(typeName) virtual TypeId type_id() const { return diesel::objectdatabase::typeids::TypeIds::##typeName; }

namespace diesel {
  namespace objectdatabase {
    typedef uint32_t RefId;
    typedef uint32_t TypeId;

    namespace typeids {
#define TYPE_ID_ENTRY(clazz, id) clazz = id,
      enum TypeIds : TypeId {
        TYPE_ID_LIST
      };
#undef TYPE_ID_ENTRY
    }

    class ReferenceMap;
    class SavingReferenceMap;

    namespace typeidclasses {
      class PersistentObject {
      public:
        PersistentObject();
        virtual ~PersistentObject();
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters);
        virtual void save(Writer& writer, SavingReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters);
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(PersistentObject);
        virtual void post_load(); // Not from diesel

        const diesel::modern::Idstring& get_name() const { return this->_name; } // Not from diesel

      private:
        diesel::modern::Idstring _name;
      };

      PersistentObject* ConstructPersistentObjectFromTypeId(TypeId typeId, diesel::EngineVersion version);

    }

    const char* TypeIdToStr(TypeId typeId);

    class ReferenceMap {
    public:
      template<typename T>
      void load_ref(RefId ref_id, T** ptr)
      {
        //if (ref_id == 1063456128)__debugbreak();
        referencesToFulfill.push_back({ ref_id, (typeidclasses::PersistentObject**)ptr });
      }

      void AddRef(RefId refId, typeidclasses::PersistentObject* obj);
      typeidclasses::PersistentObject* GetObjectFromRefId(RefId refId);

      void AssignAllReferences();

    private:
      std::unordered_map<RefId, typeidclasses::PersistentObject*> refidToObjMap;

      std::vector<std::pair<RefId, typeidclasses::PersistentObject**>> referencesToFulfill;
    };

    class SavingReferenceMap {
    public:
      void WriteRef(Writer& writer, typeidclasses::PersistentObject* obj) { this->referencesToWrite.push_back(std::make_pair(writer.GetPosition(), obj)); writer.WriteType<uint32_t>(0); }
    public:
      void AddRef(typeidclasses::PersistentObject* obj, RefId refId) { objectRefids.insert(std::make_pair(obj, refId)); }

      void WriteReferences(Writer& writer);

    private:
      std::map<typeidclasses::PersistentObject*, RefId> objectRefids;
      std::vector<std::pair<size_t, typeidclasses::PersistentObject*>> referencesToWrite;
    };

    class ObjectDatabase {
    public:
      ObjectDatabase(Reader& reader, const diesel::DieselFormatsLoadingParameters& loadParameters);
      ~ObjectDatabase();

      bool Write(Writer& writer, const diesel::DieselFormatsLoadingParameters& loadParameters);

      std::vector<typeidclasses::PersistentObject*>& GetObjects() { return _object_list; }
      const std::vector<typeidclasses::PersistentObject*>& GetObjects() const { return _object_list; }
    private:
      std::vector<typeidclasses::PersistentObject*> _object_list;
    };
  }
}