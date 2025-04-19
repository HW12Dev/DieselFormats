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
          enum PrimitiveType {};
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

        enum ChannelType : int32_t {
          CT_UNINITIALIZED = 0x0,
          CT_FLOAT1,
          CT_FLOAT2,
          CT_FLOAT3,
          CT_FLOAT4,
          CT_UBYTE4,
          CT_SHORT2,
          CT_SHORT4,
          CT_NORMPACKED3,
          CT_COUNT,
        };
        unsigned int channel_size[];
        static_assert(sizeof(ChannelType::CT_UNINITIALIZED) == 4);
        static_assert(ChannelType::CT_COUNT == 9);

        enum class VertexComponent : int32_t {
          VC_UNINITIALIZED = 0x0,
          VC_POSITION = 0x1,
          VC_NORMAL = 0x2,
          VC_POSITION1 = 0x3,
          VC_NORMAL1 = 0x4,
          VC_COLOR1 = 0x5,
          VC_COLOR2 = 0x6,
          VC_TEXCOORD0 = 0x7,
          VC_TEXCOORD1 = 0x8,
          VC_TEXCOORD2 = 0x9,
          VC_TEXCOORD3 = 0xA,
          VC_TEXCOORD4 = 0xB,
          VC_TEXCOORD5 = 0xC,
          VC_TEXCOORD6 = 0xD,
          VC_TEXCOORD7 = 0xE,
          VC_BONEINDICES1 = 0xF,
          VC_BONEINDICES2 = 0x10,
          VC_BONEWEIGHTS1 = 0x11,
          VC_BONEWEIGHTS2 = 0x12,
          VC_POINTSIZE = 0x13,
          VC_S = 0x14,
          VC_T = 0x15,
          VC_AUX1 = 0x16,
          VC_AUX2 = 0x17,
          VC_AUX3 = 0x18,
          VC_USERDEFINED0 = 0x19,
          VC_USERDEFINED1 = 0x1A,
          VC_USERDEFINED2 = 0x1B,
          VC_USERDEFINED3 = 0x1C,
          VC_USERDEFINED4 = 0x1D,
          VC_USERDEFINED5 = 0x1E,
          VC_USERDEFINED6 = 0x1F,
          VC_USERDEFINED7 = 0x20,
          VC_POSITION2 = 0x21,
          VC_NORMAL2 = 0x22,
          VC_S1 = 0x23,
          VC_T1 = 0x24,
          VC_S2 = 0x25,
          VC_T2 = 0x26,
          VC_BILLBOARD_START = 0x27,
          VC_BILLBOARD_POSITION = 0x28,
          VC_BILLBOARD_SIZE = 0x29,
          VC_BILLBOARD_UV_UL = 0x2A,
          VC_BILLBOARD_UV_LR = 0x2B,
          VC_BILLBOARD_ANGLE = 0x2C,
          VC_BILLBOARD_RIGHT = 0x2D,
          VC_BILLBOARD_DOWN = 0x2E,
          VC_BILLBOARD_NORMAL = 0x2F,
          VC_BILLBOARD_ORIGO = 0x30,
          VC_BILLBOARD_WIDTH = 0x31,
          VC_BILLBOARD_END = 0x32,
          VC_MAX = 0x33,
        };

        struct GeometryProducerChannelDesc {
          ChannelType type;
          VertexComponent component;
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

      class LightSet : public PersistentObject {};

      class Model : public Object3D {
      public:
        virtual void load(Reader& reader, ReferenceMap& ref_map, const DieselFormatsLoadingParameters& loadParameters) override;
        PERSISTENTOBJECT_VIRTUAL_FUNCTION_TYPE_ID_AUTOFILL(Model);

      private:
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
        enum Type : uint32_t {
          POINT_LIST = 0,
          LINE_LIST,
          LINE_STRIP,
          TRIANGLE_LIST,
          TRIANGLE_STRIP,
          TRIANGLE_FAN,
        };
        static_assert(sizeof(Type::POINT_LIST) == 4);
        static_assert(Type::TRIANGLE_FAN == 5);
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
        IndexProducer::Type _type;
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
        Geometry* _geometry;
        Topology* _topology;
      };

    }
  }
}