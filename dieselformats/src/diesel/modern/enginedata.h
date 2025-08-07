#pragma once

#include "diesel/shared.h"

#include "diesel/modern/hash.h"

#include <map>
#include <string>
#include <vector>

namespace diesel {
  namespace modern {
    class EngineData {
    public:
      enum class EngineValueType {
        Nil = 0,
        False = 1,
        True = 2,
        Number = 3,
        String = 4,
        Vector3 = 5,
        Quaternion = 6, // possibly?
        Idstring = 7,
        Table = 8,

      };
      static bool IsValidValueType(EngineValueType type) { return (int)type > 0 && (int)type < ((int)EngineValueType::Table)+1; }
      class Table;
      class EngineValue {
      public:
        EngineValue() : _id(0) {}
        EngineValue(unsigned int id) : _id(id) {}

        EngineValueType get_type() const { return (EngineValueType)((_id & ~0xFFFFFF) >> 24); }
        int get_index() const { return _id & 0xFFFFFF; }
      private:
        unsigned int _id;
      };
      class reference {
      public:
        reference(EngineData* data, EngineValue value) : _data(data), _value(value) {}

      public:
        EngineData* get_data() const { return _data; }
        EngineValue get_value() const { return _value; }
        Table get_table() const;

        EngineValueType get_type() const { return _value.get_type(); }
        std::string to_string() const;
        
      private:
        EngineData* _data;
        EngineValue _value;
      };
      class Table {
      public:
        bool Read(Reader& reader, const DieselFormatsLoadingParameters& version);

      public:
        Idstring _name;
        std::vector<EngineValue> _array;
        std::map<Idstring, EngineValue> _hash;
      };
    public:
      friend class reference;

      bool Read(Reader& reader, const DieselFormatsLoadingParameters& version);

      static std::string DumpReferenceToXml(const reference& ref, int indent = 0);

      reference GetRoot() { return reference(this, _root); }

      bool IsValueOutOfRangeOfArray(EngineValue value) const;

    private:
      std::vector<float> _numbers;
      std::vector<Vector3> _vector3s;
      std::vector<Quaternion> _quaternions;
      std::vector<Idstring> _idstrings;
      std::vector<Table> _tables;
      EngineValue _root;
    };
  }
}