#include "diesel/modern/massunit.h"

using namespace diesel;
using namespace diesel::modern;

MassUnitResource::MassUnitResource()
{

}


bool MassUnitResource::Read(Reader& reader, const DieselFormatsLoadingParameters& version)
{
  if (!VerifyBlobType(reader, 0xCC0BEED1))
    return false;
  auto start_pos = reader.GetPosition();
  Vector<UnitTypeData> _types(reader, version);

  reader.SetPosition(start_pos + _types._data);

  for (size_t i = 0; i < _types._size; i++) {
    UnitTypeData unit_type_data{};

    unit_type_data.unit_type = reader.ReadType<uint64_t>();
    unit_type_data.pool_size = reader.ReadType<int>();

    if (!diesel::AreLoadParameters32Bit(version)) reader.AddPosition(4);
    
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

bool MassUnitResource::Write(Writer& writer, const DieselFormatsLoadingParameters& version)
{
  size_t typesDataPosition = 0;
  Vector<UnitTypeData>::Write(writer, version, _types.size(), _types.size(), typesDataPosition);

  std::vector<std::pair<std::vector<InstanceData>*, size_t>> instanceDataPositions;

  size_t typesData = writer.GetPosition();
  for (auto& type : _types)
  {
    writer.WriteType<uint64_t>(type.unit_type);
    writer.WriteType<int>(type.pool_size);

    if (!diesel::AreLoadParameters32Bit(version)) writer.AddPosition(4);

    size_t instancesDataPosition = 0;
    Vector<InstanceData>::Write(writer, version, type.instances.size(), type.instances.size(), instancesDataPosition);

    instanceDataPositions.push_back(std::make_pair(&type.instances, instancesDataPosition));

  }

  writer.AlignToSize(16);

  for (auto& instanceData : instanceDataPositions)
  {
    size_t data = writer.GetPosition();
    for (auto& instance : *instanceData.first)
    {
      writer.WriteType<Vector3>(instance.pos);
      writer.WriteType<Quaternion>(instance.rot);
    }

    size_t returnPos = writer.GetPosition();
    writer.SetPosition(instanceData.second);
    if (diesel::AreLoadParameters32Bit(version)) { writer.WriteType<uint32_t>((uint32_t)data); } else { writer.WriteType<uint64_t>(data); }
    writer.SetPosition(returnPos);
  }

  writer.WriteType<uint32_t>(0xCC0BEED1); // type id

  writer.SetPosition(typesDataPosition);
  if (diesel::AreLoadParameters32Bit(version)) { writer.WriteType<uint32_t>((uint32_t)typesData); } else { writer.WriteType<uint64_t>(typesData); }

  return true;
}
