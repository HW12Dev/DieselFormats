#include "diesel/objectdatabase.h"

#include "diesel/modern/hashlist.h"

#include <cassert>

void diesel::typeidclasses::Model::load(Reader& reader, ReferenceMap& ref_map, diesel::EngineVersion version) { // dsl::Model::load from PAYDAY 2 Linux/PDTH v1/Lead and Gold/Ballistics decomp/RAID decomp
  Object3D::load(reader, ref_map, version);

  auto type = reader.ReadType<uint32_t>();

  if (type == 6) { // present in pd2 linux + raid
    // TODO: extra information is stored when this condition is true
    return;
  }

  auto geometryproducer_refid = reader.ReadType<RefId>();
  auto indexproducer_refid = reader.ReadType<RefId>();

  auto atom_count = reader.ReadType<uint32_t>();

  for (int i = 0; i < atom_count; i++) { // TODO: More changes for ballistics support
    auto& atom = this->_atoms.emplace_back(IndexedRenderAtom());

    if (version == EngineVersion::LEAD_AND_GOLD) {
      atom.layer = reader.ReadType<uint32_t>();
    }
    else {
      atom.layer = -1;
    }
    atom.vertex_offset = reader.ReadType<uint32_t>();
    atom.primitives = reader.ReadType<uint32_t>();
    atom.index_offset = reader.ReadType<uint32_t>();
    atom.vertices = reader.ReadType<uint32_t>();


    if (geometryproducer_refid) {
      auto& ref_value = atom.streams.emplace_back(nullptr);
      ref_map.load_ref(geometryproducer_refid, &ref_value);
    }
    if (indexproducer_refid) {
      ref_map.load_ref(indexproducer_refid, &atom.indices);
    }
  }

  reader.ReadType<RefId>(); // MaterialGroup refid


  auto lightset_refid = reader.ReadType<RefId>();
  ref_map.load_ref(lightset_refid, &this->_light_set);


  this->_properties = reader.ReadType<uint32_t>();
  this->_bounding_volume = reader.ReadType<BoundingVolume>();

  if(version != EngineVersion::BALLISTICS) // not present in ballistics (no support for skeletal meshes)
    reader.ReadType<RefId>(); // bones refid
}