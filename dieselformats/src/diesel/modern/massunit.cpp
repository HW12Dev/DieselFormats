#include "diesel/modern/massunit.h"

using namespace diesel;
using namespace diesel::modern;

MassUnitResource::MassUnitResource()
{

}


bool MassUnitResource::Read(Reader& reader, const DieselFormatsLoadingParameters& version)
{
  auto start_pos = reader.GetPosition();
  Vector<UnitTypeData> _types(reader, version);

  reader.SetPosition(start_pos + _types._data);

  for (size_t i = 0; i < _types._size; i++) {
    UnitTypeData unit_type_data{};

    unit_type_data.unit_type = reader.ReadType<uint64_t>();
    unit_type_data.pool_size = reader.ReadType<int>();
    
    Vector<InstanceData> instances(reader, version);

    reader.AddPosition(4);

    auto return_pos = reader.GetPosition();

    reader.SetPosition(start_pos + instances._data);
    for (size_t j = 0; j < instances._size; j++) {
      InstanceData instance_data{};

      instance_data.pos = reader.ReadType<Vector3>();
      instance_data.rot = reader.ReadType<Quaternion>();

      unit_type_data.instances.push_back(instance_data);
    }
    reader.SetPosition(return_pos);

    this->_types.push_back(unit_type_data);
  }

  return true;
}
