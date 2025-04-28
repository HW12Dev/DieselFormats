#include "diesel/oil.h"

namespace diesel {
  namespace oil {
    void Read_Anim_Block3(Reader& reader, const DieselFormatsLoadingParameters& version) {
      auto chunk_type = reader.ReadType<uint32_t>();
      assert(chunk_type == 20);

      auto chunk_size = reader.ReadType<uint32_t>();

      auto scene_start_time = reader.ReadType<double>();
      auto scene_end_time = reader.ReadType<double>();

      auto scene_author_tag = reader.ReadLengthPrefixedString();
      auto scene_last_export_source = reader.ReadLengthPrefixedString();

      auto scene_type = reader.ReadLengthPrefixedString();
    }

    void Read_Materials_XML(Reader& reader, const DieselFormatsLoadingParameters& version) {
      auto chunk_type = reader.ReadType<uint32_t>();
      assert(chunk_type == 11);

      auto chunk_size = reader.ReadType<uint32_t>();

      auto materials_xml = reader.ReadLengthPrefixedString();
    }

    void Read_Node(Reader& reader, const DieselFormatsLoadingParameters& version) {
      auto chunk_type = reader.ReadType<uint32_t>();
      assert(chunk_type == 0);

      auto chunk_size = reader.ReadType<uint32_t>();

    }

    OIL::OIL(Reader& reader, const DieselFormatsLoadingParameters& version) {
      auto header = reader.ReadType<uint32_t>(); // "FORM"
      assert(header == _byteswap_ulong('FORM'));

      auto total_size = reader.ReadType<uint32_t>();

      Read_Anim_Block3(reader, version);

      Read_Materials_XML(reader, version);

      oiltypes::Node rp_anim_cube;
      oiltypes::Node anim_cool;
      oiltypes::Node c_box_05;

      reader.AddPosition(8);
      rp_anim_cube.load(reader, version);
      reader.AddPosition(8);
      anim_cool.load(reader, version);
      reader.AddPosition(8);
      c_box_05.load(reader, version);

      reader.AddPosition(8);
      oiltypes::Mesh mesh;
      mesh.load(reader, version);
      //Read_Mesh(reader, version);

      __debugbreak();
    }

    OIL::~OIL() {
    }

    namespace oiltypes {
      ChunkBase* InstantiateEmptyChunkBaseFromType(ChunkType type) {
        if (type == ChunkType::Node)
          return new Node();
        if (type == ChunkType::Mesh)
          return new Mesh();
        return nullptr;
      }

      Mesh::Mesh() {
      }
      void Mesh::load(Reader& reader, const DieselFormatsLoadingParameters& version) {

        this->parent_id = reader.ReadType<int32_t>();
        this->top_material = reader.ReadType<int32_t>();

        this->cast_shadows = reader.ReadType<int8_t>();
        this->receive_shadows = reader.ReadType<int8_t>();

        auto mesh_datachannels_size = reader.ReadType<uint32_t>();

        for (int i = 0; i < mesh_datachannels_size; i++) {
          Mesh::DataChannel datachannel{};

          datachannel.type = reader.ReadType<DataChannelType>();
          datachannel.mapping_channel_index = reader.ReadType<int32_t>();
          
          auto datachannel_size = reader.ReadType<uint32_t>();

          for (int j = 0; j < datachannel_size; j++) {
            datachannel.elements.push_back(reader.ReadType<Vector3d>());
          }

          this->datachannels.push_back(datachannel);
        }

        auto mesh_number_of_faces = reader.ReadType<uint32_t>();

        for (int i = 0; i < mesh_number_of_faces; i++) {
          Mesh::Face face{};

          face.material_id = reader.ReadType<int32_t>();
          face.smoothing_group = reader.ReadType<uint32_t>();

          auto index_count = reader.ReadType<uint32_t>();

          for (int j = 0; j < index_count; j++) {
            Mesh::Face::IndexTriplet index_triplet{};

            index_triplet.channel_index = reader.ReadType<uint32_t>();
            index_triplet.index_in_channel1 = reader.ReadType<uint32_t>();
            index_triplet.index_in_channel2 = reader.ReadType<uint32_t>();
            index_triplet.index_in_channel3 = reader.ReadType<uint32_t>();

            face.indices.push_back(index_triplet);
          }

          this->faces.push_back(face);

        }

        this->skin_enable = reader.ReadType<int8_t>();

        if (this->skin_enable) {

        }

        this->override_bounding_box_enable = reader.ReadType<int8_t>();

        if (this->override_bounding_box_enable) {
          this->override_bounding_box_min_p = reader.ReadType<Vector3d>();
          this->override_bounding_box_max_p = reader.ReadType<Vector3d>();
        }
      }
      Node::Node() {
      }
      void Node::load(Reader& reader, const DieselFormatsLoadingParameters& version) {
        this->id = reader.ReadType<int32_t>();

        this->name = reader.ReadLengthPrefixedString();


        this->transform = reader.ReadType<Matrix4d>();
        this->pivot_transform = reader.ReadType<Matrix4d>();

        this->parent_id = reader.ReadType<int32_t>();
      }

    }
  }
}