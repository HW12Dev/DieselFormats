#include <assimp/scene.h>
#include <assimp/Exporter.hpp>

#include <diesel/modern/hashlist.h>
#include <diesel/objectdatabase.h>
#include <diesel/objectdatabase_model.h>

using namespace diesel;
using namespace diesel::objectdatabase;
using namespace diesel::objectdatabase::typeidclasses;

Object3D* find_root(const ObjectDatabase* objdb)
{
  for (PersistentObject* object : objdb->GetObjects()) {
    if (dynamic_cast<Object3D*>(object)) {
      if (((Object3D*)object)->get_parent() == nullptr) {
        return (Object3D*)object;
      }
    }
  }
  return nullptr;
}

struct DieselAssimpSerialiser {

  DieselAssimpSerialiser(ObjectDatabase* objdb)
  {
    assimpscene.mRootNode = new aiNode();
    assimpscene.mRootNode->mNumChildren = 1;
    assimpscene.mRootNode->mChildren = new aiNode * [assimpscene.mRootNode->mNumChildren];
    assimpscene.mRootNode->mChildren[0] = create_assimp_node(find_root(objdb), nullptr);

    assimpscene.mNumMaterials = 1;
    assimpscene.mMaterials = new aiMaterial*[assimpscene.mNumMaterials];
    assimpscene.mMaterials[0] = new aiMaterial();

    assimpscene.mNumMeshes = assimpmeshes.size();
    assimpscene.mMeshes = new aiMesh*[assimpscene.mNumMeshes];
    for (int i = 0; i < assimpmeshes.size(); i++) {
      assimpscene.mMeshes[i] = assimpmeshes[i];
    }
  }
  void export_mesh(std::string format, std::string path)
  {
    Assimp::Exporter exporter;
    exporter.Export(&assimpscene, format, path);
  }

private:
  aiScene assimpscene;
  std::map<const model::IndexedRenderAtom*, unsigned int> assimpmeshindices;
  std::vector<aiMesh*> assimpmeshes;

  unsigned int create_mesh(Model* model, const model::IndexedRenderAtom& atom)
  {
    if (assimpmeshindices.contains(&atom)) {
      return assimpmeshindices.at(&atom);
    }

    aiMesh* mesh = new aiMesh();

    int index = assimpscene.mNumMeshes;
    assimpscene.mNumMeshes++;

    assimpmeshes.push_back(mesh);
    assimpmeshindices[&atom] = index;

    //const model::IndexedRenderAtom& atom = model->_atoms[0];

    mesh->mNumVertices = atom.vertices;
    mesh->mVertices = new aiVector3D[mesh->mNumVertices];
    //mesh->mNormals = new aiVector3D[mesh->mNumVertices];
    mesh->mTextureCoords[0] = new aiVector3D[mesh->mNumVertices];
    mesh->mNumUVComponents[0] = 2;

    assert(atom.streams.size() == 1);

    if (!atom.streams[0])
      return index;

    auto producer = ((PassThroughGP*)atom.streams[0])->_geometry;
    auto topology = ((PassThroughGP*)atom.streams[0])->_topology;

    uint64_t vertexOffset = 0;
    uint64_t normalOffset = 0;
    uint64_t texCoordOffset = 0;
    model::GenericChannelType texcoordtype = model::GenericChannelType::CT_COUNT;

    for (int i = 0; i < producer->_format.size(); i++) {
      auto& format = producer->_format[i];
      if (format.component == model::GenericVertexComponent::VC_POSITION) {
        vertexOffset = producer->_channel_offset[i];
      }
      if (format.component == model::GenericVertexComponent::VC_NORMAL) {
        normalOffset = producer->_channel_offset[i];
      }
      if (format.component == model::GenericVertexComponent::VC_TEXCOORD0) {
        texCoordOffset = producer->_channel_offset[i];
        texcoordtype = format.type;
      }

    }

    unsigned int vertexCount = atom.vertices;

    for (int i = 0; i < vertexCount; i++) {

      if (producer->_vertex_size * i + vertexOffset >= producer->vertices_size) __debugbreak();

      unsigned int vertexDataStart = atom.vertex_offset * producer->_vertex_size + producer->_vertex_size * i;



      mesh->mVertices[i] = *(aiVector3D*)&producer->_vertices[vertexDataStart + vertexOffset];

      printf("%u : X: %f Y: %f Z: %f\n", i, mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

      //float y = mesh->mVertices[i].y;
      //float z = mesh->mVertices[i].z;
      //mesh->mVertices[i].y = z;
      //mesh->mVertices[i].z = y;

      if (texcoordtype == model::GenericChannelType::CT_FLOAT3) {
        mesh->mTextureCoords[0][i] = *(aiVector3D*)&producer->_vertices[producer->_vertex_size * i + texCoordOffset];
      }
      else {
        aiVector3D vec3;
        vec3.x = ((float*)&producer->_vertices[producer->_vertex_size * i + texCoordOffset])[0];
        vec3.y = ((float*)&producer->_vertices[producer->_vertex_size * i + texCoordOffset])[1];
        vec3.z = 0.f;
        mesh->mTextureCoords[0][i] = vec3;
      }
    }


    const unsigned int IndiciesPerFace = 3;

    mesh->mNumFaces = vertexCount / IndiciesPerFace;
    mesh->mFaces = new aiFace[mesh->mNumFaces]{};
    
    if (topology->_type == IndexProducer::IndexProducerType::TRIANGLE_STRIP) {
      __debugbreak();
    }

    assert(topology->_type == IndexProducer::IndexProducerType::TRIANGLE_LIST);
    assert(atom.index_offset == 0);
    
    for (int i = 0; i < mesh->mNumFaces; i++) {
      aiFace& face = mesh->mFaces[i];


      unsigned int indexPosition = atom.index_offset + i * IndiciesPerFace;

      face.mNumIndices = IndiciesPerFace;
      face.mIndices = new unsigned int[face.mNumIndices] {};
      face.mIndices[0] = (unsigned int)topology->_indices[indexPosition + 0];
      face.mIndices[1] = (unsigned int)topology->_indices[indexPosition + 1];
      face.mIndices[2] = (unsigned int)topology->_indices[indexPosition + 2];

      assert(face.mIndices[0] < atom.vertices);
      assert(face.mIndices[1] < atom.vertices);
      assert(face.mIndices[2] < atom.vertices);

    }

    return index;
  }

  aiNode* create_assimp_node(Object3D* object, aiNode* parent)
  {
    aiNode* thiz = new aiNode();

    const char* target_name = "g_gun_case";

    diesel::modern::GetGlobalHashlist()->AddSourceToHashlist(target_name);

    thiz->mName = modern::GetGlobalHashlist()->GetIdstringSource(object->get_name());
    thiz->mParent = parent;

    thiz->mTransformation = *(aiMatrix4x4*)&object->get_local_tm();

    thiz->mNumChildren = (unsigned int)object->get_children().size();
    thiz->mChildren = new aiNode * [thiz->mNumChildren];
    for (int i = 0; i < object->get_children().size(); i++) {
      thiz->mChildren[i] = create_assimp_node(object->get_children()[i], thiz);
    }
    if (object->get_name() != diesel::modern::Idstring(target_name)) {
      return thiz;
    }

    if (object->type_id() == typeids::Model) {
      thiz->mNumMeshes = ((Model*)object)->_atoms.size();
      thiz->mMeshes = new unsigned int[thiz->mNumMeshes] {};

      for (int i = 0; i < ((Model*)object)->_atoms.size(); i++) {
        thiz->mMeshes[i] = create_mesh((Model*)object, ((Model*)object)->_atoms[i]);
      }

    }

    return thiz;
  }

};

void hashlist()
{
  Reader hashlist(R"(X:\Projects\DieselEngineExplorer\hashlist.txt)");
  modern::GetGlobalHashlist()->ReadFileToHashlist(hashlist);
}

int main()
{
  //hashlist();
  Reader objdbreader(R"(X:\Projects\SafeOpening\payday2_cash\safe_room\cash_int_safehouse_saferoom.model)");
  //Reader objdbreader(R"(X:\Projects\SafeOpening\payday2_cash\safes\ait\safe\eco_safe_ait.model)");

  ObjectDatabase* dieselmesh = new ObjectDatabase(objdbreader, diesel::DieselFormatsLoadingParameters(diesel::EngineVersion::PAYDAY_2_LATEST));

  DieselAssimpSerialiser serialiser(dieselmesh);
  serialiser.export_mesh("fbx", R"(X:\Projects\SafeOpening\payday2_cash\safe_room\cash_int_safehouse_saferoom.fbx)");
  //serialiser.export_mesh("fbx", R"(X:\Projects\SafeOpening\payday2_cash\safes\ait\safe\eco_safe_ait.fbx)");

  return 0;
}