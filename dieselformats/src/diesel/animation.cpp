#include "diesel/animation.h"

#include "fileio/zlibcompression.h"


#include "fileio/writer.h"

using namespace diesel;

bool Animation::Read(Reader& reader, const DieselFormatsLoadingParameters& version) {
  size_t compressedBufferSize = reader.GetFileSize() - 4;

  size_t start = reader.GetPosition();

  reader.AddPosition(compressedBufferSize);

  size_t uncompressedBufferSize = reader.ReadType<uint32_t>();

  reader.SetPosition(start);

  char* uncompressedBuffer = new char[uncompressedBufferSize] {};
  char* compressedBuffer = new char[compressedBufferSize] {};

  reader.ReadBytesToBuffer(compressedBuffer, compressedBufferSize);

  compression::ZlibDecompression::DecompressBuffer(compressedBuffer, compressedBufferSize, uncompressedBuffer, uncompressedBufferSize);

  delete[] compressedBuffer;

  Reader uncompressedReader(uncompressedBuffer, uncompressedBufferSize);
  

  return ReadUncompressed(uncompressedReader, version);
}



bool Animation::ReadUncompressed(Reader& reader, const DieselFormatsLoadingParameters& version) {
  size_t start = reader.GetPosition();

  {
      // BlobHeader

      
      uint32_t type_id = reader.ReadType<uint32_t>();

      if (type_id != 0x0883CC85) // AnimationBlob
      {
          return false;
      }

      if (diesel::AreLoadParameters32Bit(version))
      {
          reader.AddPosition(8);

          // version
          // original_location
      }
      else
      {
          reader.AddPosition(12);
      }
      size_t size = reader.ReadType<uint32_t>();

      if (!diesel::AreLoadParameters32Bit(version))
      {
          reader.AddPosition(12);
      }
  }

  _length = reader.ReadType<float>();

  if (!diesel::AreLoadParameters32Bit(version)) {
    reader.AddPosition(4); // class alignment
  }

  InplaceArray<InplaceString> names(reader, version);
  InplaceArray<BeatTime> beat_times(reader, version);
  InplaceArray<AnimationTrigger> trigger_times(reader, version);
  InplaceArray<InplaceVirtual<PositionData>> positions(reader, version);
  InplaceArray<InplaceVirtual<RotationData>> rotations(reader, version);

  reader.SetPosition(start + names._data);

  for (int i = 0; i < names._n; i++) {
    InplaceString str(reader, version);

    size_t returnPos = reader.GetPosition();

    reader.SetPosition(start + str._s);

    _names.push_back(reader.ReadString());

    reader.SetPosition(returnPos);
    
  }

  reader.SetPosition(start + beat_times._data);

  for (int i = 0; i < beat_times._n; i++)
  {
      BeatTime beat_time;

      beat_time.time = reader.ReadType<float>();

      if (!diesel::AreLoadParameters32Bit(version))
	  {
		  reader.AddPosition(4); // alignment
	  }

      InplaceString name(reader, version);

      size_t returnPos = reader.GetPosition();

      reader.SetPosition(start + name._s);

      beat_time.name = reader.ReadString();

      reader.SetPosition(returnPos);

      _beat_times.push_back(beat_time);
  }

  reader.SetPosition(start + trigger_times._data);

  for (int i = 0; i < trigger_times._n; i++)
  {
      AnimationTrigger beat_time;

      beat_time.time = reader.ReadType<float>();

      if (!diesel::AreLoadParameters32Bit(version))
	  {
		  reader.AddPosition(4); // alignment
      }

      InplaceString name(reader, version);

      size_t returnPos = reader.GetPosition();

      reader.SetPosition(start + name._s);

      beat_time.name = reader.ReadString();

      reader.SetPosition(returnPos);

      _trigger_times.push_back(beat_time);
  }

  AnimationParsingScope inplace_virtual_scope{};
  inplace_virtual_scope.start = start;

  reader.SetPosition(start + positions._data);

  for (int i = 0; i < positions._n; i++)
  {
      InplaceVirtual<PositionData> data(reader, version);

      std::shared_ptr<PositionData> object;

      switch (data._type_id)
      {
      case AnimationTypeId::AnimationTypeId_HermitePositionData:
          object = std::make_shared<HermitePositionData>();
          break;
      case AnimationTypeId::AnimationTypeId_PackedHermitePositionData:
          object = std::make_shared<PackedHermitePositionData>();
          break;
      case AnimationTypeId::AnimationTypeId_SampledPositionData:
          object = std::make_shared<SampledPositionData>();
          break;
      }



      size_t returnPos = reader.GetPosition();
      if (object)
      {
          reader.SetPosition(data._object);
          object->Read(reader, version, inplace_virtual_scope);
          _positions.push_back(object);
      }
      reader.SetPosition(returnPos);
  }

  reader.SetPosition(start + rotations._data);

  for (int i = 0; i < rotations._n; i++)
  {
      InplaceVirtual<RotationData> data(reader, version);

      std::shared_ptr<RotationData> object;

      switch (data._type_id)
      {
      case AnimationTypeId::AnimationTypeId_HermiteRotationData:
          object = std::make_shared<HermiteRotationData>();
          break;
      case AnimationTypeId::AnimationTypeId_PackedHermiteRotationData:
          object = std::make_shared<PackedHermiteRotationData>();
          break;
      case AnimationTypeId::AnimationTypeId_SampledRotationData:
          object = std::make_shared<SampledRotationData>();
          break;
      }



      size_t returnPos = reader.GetPosition();
      if (object)
      {
          reader.SetPosition(data._object);
          object->Read(reader, version, inplace_virtual_scope);
          _rotations.push_back(object);
      }
      reader.SetPosition(returnPos);

  }

  return true;
}


#define WRITE_POSITION_BACK(position, data) \
    writer.SetPosition(position); \
    if (diesel::AreLoadParameters32Bit(version)) { writer.WriteType<uint32_t>((uint32_t)data); } else { writer.WriteType<uint64_t>(data); }


bool Animation::Write(Writer& writer, const DieselFormatsLoadingParameters& version)
{
    writer.WriteType(0x0883CC85); // AnimationBlob

    if (diesel::AreLoadParameters32Bit(version))
    {
        writer.AddPosition(8);
    }
    else
    {
        writer.AddPosition(12);
    }

    size_t sizePosition = writer.GetPosition();
    writer.WriteType<uint32_t>(0); // always 32bit int

    if (!diesel::AreLoadParameters32Bit(version))
    {
        writer.AddPosition(12);
    }

    writer.WriteType<float>(_length);

    if (!diesel::AreLoadParameters32Bit(version))
    {
        writer.AddPosition(4);
    }
    
    size_t namesDataPosition = 0;
    InplaceArray<InplaceString>::Write(writer, version, _names.size(), namesDataPosition);
    size_t beatTimesDataPosition = 0;
    InplaceArray<BeatTime>::Write(writer, version, _beat_times.size(), beatTimesDataPosition);
    size_t triggerTimesDataPosition = 0;
    InplaceArray<AnimationTrigger>::Write(writer, version, _trigger_times.size(), triggerTimesDataPosition);
    size_t positionsDataPosition = 0;
    InplaceArray<InplaceVirtual<PositionData>>::Write(writer, version, _positions.size(), positionsDataPosition);
    size_t rotationsDataPosition = 0;
    InplaceArray<InplaceVirtual<RotationData>>::Write(writer, version, _rotations.size(), rotationsDataPosition);

    size_t namesData = writer.GetPosition();
    {
        std::vector<std::pair<std::string, uint64_t>> namesDataPositions;
        size_t namesData = writer.GetPosition();

        for (size_t i = 0; i < _names.size(); i++)
        {
            size_t dataPosition = 0;
            InplaceString::Write(writer, version, dataPosition);

            namesDataPositions.push_back(std::make_pair(_names[i], dataPosition));
        }

        for (auto& stringsToWrite : namesDataPositions)
        {
            size_t stringStart = writer.GetPosition();
            writer.WriteString(stringsToWrite.first);

            size_t saved = writer.GetPosition();
            WRITE_POSITION_BACK(stringsToWrite.second, stringStart);
            writer.SetPosition(saved);
        }
    }

    size_t beatTimesData = writer.GetPosition();
    {
        std::vector<std::pair<std::string, uint64_t>> namesDataPositions;
        size_t namesData = writer.GetPosition();

        for (size_t i = 0; i < _beat_times.size(); i++)
        {
            size_t dataPosition = 0;

            writer.WriteType<float>(_beat_times[i].time);
            
            if (!diesel::AreLoadParameters32Bit(version))
			{
				writer.AddPosition(4); // alignment
			}

            InplaceString::Write(writer, version, dataPosition);

            namesDataPositions.push_back(std::make_pair(_beat_times[i].name, dataPosition));
        }

        for (auto& stringsToWrite : namesDataPositions)
        {
            size_t stringStart = writer.GetPosition();
            writer.WriteString(stringsToWrite.first);

            size_t saved = writer.GetPosition();
            WRITE_POSITION_BACK(stringsToWrite.second, stringStart);
            writer.SetPosition(saved);
        }
    }

    size_t triggerTimesData = writer.GetPosition();
    {
        std::vector<std::pair<std::string, uint64_t>> namesDataPositions;
        size_t namesData = writer.GetPosition();

        for (size_t i = 0; i < _trigger_times.size(); i++)
        {
            size_t dataPosition = 0;

            writer.WriteType<float>(_trigger_times[i].time);

            if (!diesel::AreLoadParameters32Bit(version))
			{
				writer.AddPosition(4); // alignment
			}

            InplaceString::Write(writer, version, dataPosition);

            namesDataPositions.push_back(std::make_pair(_trigger_times[i].name, dataPosition));
        }

        for (auto& stringsToWrite : namesDataPositions)
        {
            size_t stringStart = writer.GetPosition();
            writer.WriteString(stringsToWrite.first);

            size_t saved = writer.GetPosition();
            WRITE_POSITION_BACK(stringsToWrite.second, stringStart);
            writer.SetPosition(saved);
        }
    }

    writer.AlignToSize(8);

    AnimationParsingScope inplace_virtual_scope{};

    size_t positionsData = writer.GetPosition();
    {
        std::vector<std::pair<PositionData*, size_t>> positionPositions;
        for (auto& position : _positions)
        {
            size_t virtualDataPosition = 0;
            InplaceVirtual<PositionData>::Write(writer, version, position->type_id(), virtualDataPosition);

            positionPositions.push_back(std::make_pair(position.get(), virtualDataPosition));
        }

        
        for (auto& position : positionPositions)
        {
            size_t dataPos = writer.GetPosition();

            position.first->Write(writer, version, inplace_virtual_scope);

            size_t returnPos = writer.GetPosition();
            WRITE_POSITION_BACK(position.second, dataPos);
            writer.SetPosition(returnPos);

        }
        
    }

    size_t rotationsData = writer.GetPosition();
    {
        std::vector<std::pair<RotationData*, size_t>> rotationPositions;
        for (auto& position : _rotations)
        {
            size_t virtualDataPosition = 0;
            InplaceVirtual<RotationData>::Write(writer, version, position->type_id(), virtualDataPosition);

            rotationPositions.push_back(std::make_pair(position.get(), virtualDataPosition));
        }

        for (auto& position : rotationPositions)
        {
            size_t dataPos = writer.GetPosition();

            position.first->Write(writer, version, inplace_virtual_scope);

            size_t returnPos = writer.GetPosition();
            WRITE_POSITION_BACK(position.second, dataPos);
            writer.SetPosition(returnPos);
        }
    }

    size_t totalSize = writer.GetPosition();

    WRITE_POSITION_BACK(namesDataPosition, namesData);
    WRITE_POSITION_BACK(beatTimesDataPosition, beatTimesData);
    WRITE_POSITION_BACK(triggerTimesDataPosition, triggerTimesData);
    WRITE_POSITION_BACK(positionsDataPosition, positionsData);
    WRITE_POSITION_BACK(rotationsDataPosition, rotationsData);

    WRITE_POSITION_BACK(sizePosition, totalSize);

    return true;
}

#define ANIMATION_DATA_READ(clazz, parent) \
bool Animation::clazz::Read(Reader& reader, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope) \
{ \
    parent::Read(reader, version, scope); \
\
    InplaceArray<Sample> data(reader, version); \
\
    reader.SetPosition(scope.start + data._data); \
\
    for (int i = 0; i < data._n; i++) \
    { \
        Sample sample = reader.ReadType<Sample>(); \
\
        _data.push_back(sample); \
    } \
\
    return true; \
}

#define ANIMATION_DATA_WRITE(clazz, parent) \
bool Animation::clazz::Write(Writer& writer, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope) \
{ \
    parent::Write(writer, version, scope); \
\
    size_t dataPosition = 0; \
    InplaceArray<Sample>::Write(writer, version, _data.size(), dataPosition); \
\
\
    size_t sampleData = writer.GetPosition(); \
\
    for (auto& sample : _data) \
    { \
        writer.WriteType<Sample>(sample); \
    } \
\
    size_t returnPos = writer.GetPosition(); \
\
    writer.SetPosition(dataPosition); \
\
    if (AreLoadParameters32Bit(version)) \
    { \
        writer.WriteType<uint32_t>((uint32_t)sampleData); \
    } \
    else \
    { \
        writer.WriteType<uint64_t>(sampleData); \
    } \
\
    writer.SetPosition(returnPos); \
\
    return true; \
}

ANIMATION_DATA_READ(HermitePositionData, PositionData);
ANIMATION_DATA_READ(PackedHermitePositionData, PositionData);
ANIMATION_DATA_READ(SampledPositionData, PositionData);

ANIMATION_DATA_READ(HermiteRotationData, RotationData);
ANIMATION_DATA_READ(PackedHermiteRotationData, RotationData);
ANIMATION_DATA_READ(SampledRotationData, RotationData);

ANIMATION_DATA_WRITE(HermitePositionData, PositionData);
ANIMATION_DATA_WRITE(PackedHermitePositionData, PositionData);
ANIMATION_DATA_WRITE(SampledPositionData, PositionData);

ANIMATION_DATA_WRITE(HermiteRotationData, RotationData);
ANIMATION_DATA_WRITE(PackedHermiteRotationData, RotationData);
ANIMATION_DATA_WRITE(SampledRotationData, RotationData);


