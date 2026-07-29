#pragma once

#include "diesel/shared.h"

#include "fileio/reader.h"
#include "fileio/writer.h"

#include <vector>
#include <string>

namespace diesel {
  class Animation {
  public:
    bool Read(Reader& reader, const DieselFormatsLoadingParameters& version);
    bool ReadUncompressed(Reader& reader, const DieselFormatsLoadingParameters& version);

    bool Write(Writer& writer, const DieselFormatsLoadingParameters& version);

    struct BeatTime
    {
        float time;
        std::string name;
    };
    struct AnimationTrigger
    {
        float time;
        std::string name;
    };

    // shared scope for both writing and reading actions, mainly used to pass data into RotationData and PositionData children
    struct AnimationParsingScope
    {
        // read
        size_t start;

    };

    enum AnimationTypeId : uint32_t
    {
        AnimationTypeId_PositionData = 0x29968716,
        AnimationTypeId_RotationData = 0x538030DD,

        AnimationTypeId_HermiteRotationData = 0x9DFB92B6,
        AnimationTypeId_PackedHermiteRotationData = 0x96ECB85C,
        AnimationTypeId_SampledRotationData = 0xE91A69BA,

        AnimationTypeId_HermitePositionData = 0x9D3DE40D,
        AnimationTypeId_PackedHermitePositionData = 0x1DC686E0,
        AnimationTypeId_SampledPositionData = 0x1196CFB2
    };

    class PositionData
    {
    public:
        virtual uint32_t type_id() const { return AnimationTypeId::AnimationTypeId_PositionData; }

        virtual bool Read(Reader& reader, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope)
        {
            // vtable!!!!
            if (AreLoadParameters32Bit(version)) { reader.AddPosition(4); } else { reader.AddPosition(8); }
            return true;
        }
        virtual bool Write(Writer& writer, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope)
        {
            // vtable!!!!
            if (AreLoadParameters32Bit(version)) { writer.AddPosition(4); } else { writer.AddPosition(8); }
            return true;
        }
    };

    class RotationData
    {
    public:
        virtual uint32_t type_id() const { return AnimationTypeId::AnimationTypeId_RotationData; }

        virtual bool Read(Reader& reader, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope)
        {
            // vtable!!!!
            if (AreLoadParameters32Bit(version)) { reader.AddPosition(4); } else { reader.AddPosition(8); }
            return true;
        }
        virtual bool Write(Writer& writer, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope)
        {
            // vtable!!!!
            if (AreLoadParameters32Bit(version)) { writer.AddPosition(4); } else { writer.AddPosition(8); }
            return true;
        }
    };

    class HermitePositionData : public PositionData
    {
    public:
        struct Sample
        {
            float t;
            Vector3 value;
        };

        virtual uint32_t type_id() const { return AnimationTypeId::AnimationTypeId_HermitePositionData; }

        virtual bool Read(Reader& reader, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope);
        virtual bool Write(Writer& writer, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope);
    private:
        std::vector<Sample> _data;
    };

    class PackedHermitePositionData : public PositionData
    {
    public:
        struct Sample
        {
            float t;
            Vector3Packed value;
        };

        virtual uint32_t type_id() const { return AnimationTypeId::AnimationTypeId_PackedHermitePositionData; }

        virtual bool Read(Reader& reader, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope);
        virtual bool Write(Writer& writer, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope);
    private:
        std::vector<Sample> _data;
    };
    class SampledPositionData : public PositionData
    {
    public:
        struct Sample
        {
            float t;
            Vector3 value;
            bool step;
        };

        virtual uint32_t type_id() const { return AnimationTypeId::AnimationTypeId_SampledPositionData; }

        virtual bool Read(Reader& reader, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope);
        virtual bool Write(Writer& writer, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope);
    private:
        std::vector<Sample> _data;
    };

    class HermiteRotationData : public RotationData
    {
    public:
        struct Sample
        {
            float t;
            Quaternion value;
        };

        virtual uint32_t type_id() const { return AnimationTypeId::AnimationTypeId_HermiteRotationData; }

        virtual bool Read(Reader& reader, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope);
        virtual bool Write(Writer& writer, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope);
    private:
        std::vector<Sample> _data;
    };

    class PackedHermiteRotationData : public RotationData
    {
    public:
        struct Sample
        {
            float t;
            QuaternionPacked value;
        };

        virtual uint32_t type_id() const { return AnimationTypeId::AnimationTypeId_PackedHermiteRotationData; }

        virtual bool Read(Reader& reader, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope);
        virtual bool Write(Writer& writer, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope);
    private:
        std::vector<Sample> _data;
    };
    class SampledRotationData : public RotationData
    {
    public:
        struct Sample
        {
            float t;
            Quaternion value;
            bool step;
        };

        virtual uint32_t type_id() const { return AnimationTypeId::AnimationTypeId_SampledRotationData; }

        virtual bool Read(Reader& reader, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope);
        virtual bool Write(Writer& writer, const DieselFormatsLoadingParameters& version, AnimationParsingScope& scope);
    private:
        std::vector<Sample> _data;
    };

  private:
    float _length;
    std::vector<std::string> _names;
    std::vector<BeatTime> _beat_times;
    std::vector<AnimationTrigger> _trigger_times;
    std::vector<std::shared_ptr<PositionData>> _positions;
    std::vector<std::shared_ptr<RotationData>> _rotations;
  };
}