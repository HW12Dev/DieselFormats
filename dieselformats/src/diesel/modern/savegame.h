#pragma once

#include "diesel/shared.h"
#include "diesel/modern/hash.h"

#include "fileio/reader.h"

#include <variant>
#include <map>

namespace diesel {
  namespace modern {
    enum class ScriptValueTypeId : uint32_t {
      ScriptVector3 = 0x2B4DAB5,
      ScriptQuaternion = 0x80A63668,
      ScriptPolar = 0xE266CFD6,
      ScriptColor = 0x9CF6D6B8,
      ScriptIdstring = 0xAE055CEA,
      SlotMask = 0x362752E8,

    };

    class SaveGame {
    public:
      class InformationData {
        enum SerializerTypeId : uint8_t {
          _TYPE_ID_NIL = 0,
          _TYPE_ID_STRING = 1,
          _TYPE_ID_NUMBER = 2,
          _TYPE_ID_ZERO = 3,
          _TYPE_ID_INT8 = 4,
          _TYPE_ID_INT16 = 5,
          _TYPE_ID_BOOLEAN = 6,
          _TYPE_ID_TABLE = 7,

          _TYPE_ID_SCRIPTVALUE = 9,
          _TYPE_ID_SCRIPTREFERENCE = 10
        };

        class SerializerScriptValueVariant {
          friend class InformationData;
        public:


          bool Read(Reader& reader, const DieselFormatsLoadingParameters& version);

        private:
            Vector3 vector3Value;
            Quaternion quaternionValue;
            Idstring idstringValue;
          union {
            float polarValue[3];
            float colorValue[4];
            char slotMaskValue[12];
          };
          ScriptValueTypeId type;
        };
        class SerializerVariant {
          friend class InformationData;
        public:
          bool operator<(const SerializerVariant& other) const;
        private:


          SerializerTypeId GetType() const { return type; }

          void SetType(SerializerTypeId newType) { type = newType; }

          int& GetInt() { return intValue; }
          const int& GetInt() const { return intValue; }
          float& GetFloat() { return floatValue; }
          const float& GetFloat() const { return floatValue; }
          std::string& GetString() { return stringValue; }
          const std::string& GetString() const { return stringValue; }
          std::map<SerializerVariant, SerializerVariant>& GetTable() { return tableValue; }
          const std::map<SerializerVariant, SerializerVariant>& GetTable() const { return tableValue; }

          SerializerScriptValueVariant& GetScriptValue() { return scriptValue; }
          const SerializerScriptValueVariant& GetScriptValue() const { return scriptValue; }

        private:
          SerializerTypeId type;

          union {
            int intValue;
            float floatValue;
          };
          std::string stringValue;
          std::map<SerializerVariant, SerializerVariant> tableValue;
          SerializerScriptValueVariant scriptValue;
        };
      public:
        bool Read(Reader& reader, const DieselFormatsLoadingParameters& version);

      private:
        void ReadValue(Reader& reader, const DieselFormatsLoadingParameters& version, SerializerVariant& returnValue);
        void ReadTable(Reader& reader, const DieselFormatsLoadingParameters& version, SerializerVariant& returnTable);

      private:
        SerializerVariant value;
      };


    public:
      // Reads a Diesel save file, input must be the unencrypted file. PAYDAY: The Heist doesn't encrypt save games.
      bool Read(Reader& reader, const DieselFormatsLoadingParameters& version);

    private:
      InformationData informationData;
    };
  }
}