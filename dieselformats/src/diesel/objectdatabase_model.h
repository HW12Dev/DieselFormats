#pragma once

#include "diesel/objectdatabase.h"

namespace diesel {
  namespace objectdatabase {
    namespace typeidclasses {

      class GeometryProducer;
      class IndexProducer;
      class Material;

      namespace model {
        class RenderAtom {
        public:
          enum PrimitiveType : uint32_t{
            POINT_LIST = 0,
            LINE_LIST,
            LINE_STRIP,
            TRIANGLE_LIST,
            TRIANGLE_STRIP,
            TRIANGLE_FAN,
          };
        public:
          int layer;
          float sort_depth;
          RenderAtom::PrimitiveType primitive_type;
          void* render_template;
          void* shader_parameters;
          std::vector<GeometryProducer*> streams;
          unsigned int vertex_offset;
          unsigned int primitives;
          diesel::modern::Idstring tag;
          RenderAtom* child;
        };
        class IndexedRenderAtom : public RenderAtom {
        public:
          IndexProducer* indices;
          unsigned int index_offset;
          unsigned int vertices;
        };

        enum GenericChannelType : int32_t {
          CT_UNINITIALIZED = 0x0,
          CT_FLOAT1,
          CT_FLOAT2,
          CT_FLOAT3,
          CT_FLOAT4,
          CT_UBYTE4,
          CT_SHORT2,
          CT_SHORT4,
          CT_NORMPACKED3,

          CT_HALFFLOAT, // Added by RAID, used for UVs

          CT_COUNT,
        };

        int GetSizeForChannelType(GenericChannelType channelType, const diesel::DieselFormatsLoadingParameters& loadParameters);

        enum class GenericVertexComponent : int32_t {
          VC_UNINITIALIZED = 0,
          VC_POSITION,
          VC_NORMAL,
          VC_POSITION1,
          VC_NORMAL1,
          VC_COLOR1,
          VC_COLOR2,
          VC_TEXCOORD0,
          VC_TEXCOORD1,
          VC_TEXCOORD2,
          VC_TEXCOORD3,
          VC_TEXCOORD4,
          VC_TEXCOORD5,
          VC_TEXCOORD6,
          VC_TEXCOORD7,
          VC_TEXCOORD8, // Added by RAID, only available in RAID
          VC_TEXCOORD9, // Added by RAID, only available in RAID
          VC_BONEINDICES1,
          VC_BONEINDICES2,
          VC_BONEWEIGHTS1,
          VC_BONEWEIGHTS2,
          VC_POINTSIZE,
          VC_S,
          VC_T,
          VC_AUX1,
          VC_AUX2,
          VC_AUX3,
          VC_USERDEFINED0,
          VC_USERDEFINED1,
          VC_USERDEFINED2,
          VC_USERDEFINED3,
          VC_USERDEFINED4,
          VC_USERDEFINED5,
          VC_USERDEFINED6,
          VC_USERDEFINED7,
          VC_POSITION2,
          VC_NORMAL2,
          VC_S1,
          VC_T1,
          VC_S2,
          VC_T2,
          VC_BILLBOARD_START,
          VC_BILLBOARD_POSITION,
          VC_BILLBOARD_SIZE,
          VC_BILLBOARD_UV_UL,
          VC_BILLBOARD_UV_LR,
          VC_BILLBOARD_ANGLE,
          VC_BILLBOARD_RIGHT,
          VC_BILLBOARD_DOWN,
          VC_BILLBOARD_NORMAL,
          VC_BILLBOARD_ORIGO,
          VC_BILLBOARD_WIDTH,
          VC_BILLBOARD_END,
          VC_MAX,
        };

        struct GeometryProducerChannelDesc {
          GenericChannelType type;
          GenericVertexComponent component;
        };

        class BoundingVolume {
        public:
          Vector3 box_min;
          Vector3 box_max;
          float sphere_radius;
          unsigned int validity;
        };
        static_assert(sizeof(BoundingVolume) == 0x20);
      }

      class AuthorTag : public PersistentObject {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(AuthorTag);

      private:
        std::string _author_tag;
        std::string _last_export_source;
        uint32_t _version;
      };

      class Animatable : public PersistentObject {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(Animatable);

      private:
      };

      class Object3D : public Animatable {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(Object3D);

      private:
        Object3D* _parent;
        Matrix4 _local_tm;
      };

      class Light : public PersistentObject {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(Light);
      };

      class LightSet : public PersistentObject {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(LightSet);

      private:
      public:
        std::vector<Light*> lights; // contained in LightSet->_interface
      };

      class Model : public Object3D {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(Model);

      private:
      public: // TODO: REMOVE
        std::vector<model::IndexedRenderAtom> _atoms;
        std::vector<Material*> _materials;
        LightSet* _light_set;


        model::BoundingVolume _bounding_volume;
        unsigned int _properties;
      };

      class Material : public PersistentObject {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(Material);

      private:
      };

      class MaterialGroup : public PersistentObject {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(MaterialGroup);

      private:
        std::vector<Material*> _materials;
      };

      class AnimationData : public PersistentObject {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(AnimationData);

      private:
        float start_time;
        float end_time;
        std::vector<Animatable*> _animatable_list;
      };



      class IndexProducer : public PersistentObject {
      public:
        typedef model::RenderAtom::PrimitiveType IndexProducerType;
        static_assert(sizeof(IndexProducerType::POINT_LIST) == 4);
        static_assert(IndexProducerType::TRIANGLE_FAN == 5);
      public:
        //virtual void produce(...);
      };

      class GeometryProducer : public PersistentObject {
      public:
        //virtual void produce(...);
      };
      class UVDuplicationBase : public GeometryProducer {};

      class Geometry : public PersistentObject {
      public:
        ~Geometry();
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(Geometry);

      private:
      public: // TODO: REMOVE
        std::vector<model::GeometryProducerChannelDesc> _format;
        unsigned int _vertex_size;
        std::vector<unsigned int> _channel_offset;
        char* _vertices;
      };
      class Topology : public PersistentObject { // class structure from PAYDAY: The Heist v1
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(Topology);

      private:
      public: // TODO: REMOVE
        IndexProducer::IndexProducerType _type;
        std::vector<uint16_t> _indices;
        std::vector<uint8_t> _groupings;
      };

      // Topology Index Producer
      class TopologyIP : public PersistentObject {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(TopologyIP);

      private:
        Topology* _topology;
      };

      // Passthrough Geometry Producer
      class PassThroughGP : public UVDuplicationBase {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(PassThroughGP);

      private:
      public: // TODO: REMOVE
        Geometry* _geometry;
        Topology* _topology;
      };

      class Bones : public PersistentObject {
      public:
        class BoneMapping {
        public:
          std::vector<std::vector<int>> _bone_sets;
          std::vector<std::vector<Matrix4>> _matrix_sets;
        };
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        //PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(Bones);

      public:
        BoneMapping _default_bone_mapping;
      };

      class Version {
      public:
        unsigned int _v;
      };

      class SkinBones : public Bones {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(SkinBones);

      private:
      public:
        Object3D* _root_node;
        std::vector<Object3D*> _bone_nodes;

        std::vector<Matrix4> _premul_tms;
        std::vector<Matrix4> _bone_transforms;
        std::vector<Version> _versions;

        Matrix4 _postmul_tms;
      };

    }
  }
}