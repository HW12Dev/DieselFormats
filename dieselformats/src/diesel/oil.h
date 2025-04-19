#pragma once

#include "fileio/reader.h"
#include "diesel/modern/hash.h"
#include "diesel/shared.h"

#include <vector>

namespace diesel {
  namespace oil {
    enum class ChunkType : uint32_t {
      Node = 0,
      Position_Controller = 1,
      Rotation_Controller = 2,
      Anim_Block = 3,
      Material = 4,
      Mesh = 5,
      Lookat_Controller = 6,
      Color_Controller = 7,
      Attenuation_Controller = 8,
      Multiplier_Controller = 9,
      Light = 10,
      Materials_XML = 11,
      Anim_Block2 = 12,
      Hotspot_Controller = 13,
      Falloff_Controller = 14,
      FOV_Controller = 15,
      Far_Clip_Controller = 16,
      Near_Clip_Controller = 17,
      Target_Dist_Controller = 18,
      Camera = 19,
      Anim_Block3 = 20,
      Key_Event_List = 21,
      IK_Chain_Controller = 22,
      IK_Chain_Target_Controller = 23,
      Composite_Position_Controller = 24,
      Composite_Rotation_Controller = 25,
    };

    enum class DataChannelType : int32_t {
      Vertex,
      Texture_Vertex,
      Normal,
      Binormal,
      Tangent,
      Color,
      Alpha,
      Illumination,
      Unique_Vertex
    };

    namespace oiltypes {
      class Node {
      public:
        Node();
        void load(Reader& reader, const DieselFormatsLoadingParameters& version);

      public:
        int32_t id;
        std::string name;
        Matrix4d transform;
        Matrix4d pivot_transform;
        int32_t parent_id;
      };
      class Mesh {
      public:
        struct DataChannel {
          DataChannelType type;
          int32_t mapping_channel_index;
          std::vector<Vector3d> elements;
        };
        struct Face {
          struct IndexTriplet {
            uint32_t channel_index;
            uint32_t index_in_channel1;
            uint32_t index_in_channel2;
            uint32_t index_in_channel3;
          };

          int32_t material_id;
          uint32_t smoothing_group;
          std::vector<IndexTriplet> indices;
        };
      public:
        Mesh();
        void load(Reader& reader, const DieselFormatsLoadingParameters& version);

      public:
        int32_t parent_id;
        int32_t top_material;
        bool cast_shadows;
        bool receive_shadows;
        
        std::vector<DataChannel> datachannels;
        std::vector<Face> faces;

        bool skin_enable;

        bool override_bounding_box_enable;
        Vector3d override_bounding_box_min_p;
        Vector3d override_bounding_box_max_p;
      };
    }

    class OIL {
    public:
      OIL(Reader& reader, const DieselFormatsLoadingParameters& version);
      ~OIL();
    };
  }
}