#pragma once

#include <rapidjson/writer.h>
#include <map>
#include <string>
#include <vector>

namespace hvs {
class JsonSerializer {
 public:
  // 每次调用都会重新生成文档，并没有重用
  std::string serialize() {
    _writer->StartObject();
    serialize_impl();
    _writer->EndObject();
    std::string document(_buffer->GetString());
    _buffer->Clear();
    _writer->Reset(*_buffer);
    return document;
  }

 public:
  JsonSerializer() {
    _buffer = new rapidjson::StringBuffer();
    _writer = new rapidjson::Writer<rapidjson::StringBuffer>(*_buffer);
  }
  JsonSerializer(JsonSerializer& oths) {
    _writer = oths._writer;
    _buffer = oths._buffer;
  }
  ~JsonSerializer() { delete _writer; delete _buffer;}

 protected:
  template <class T>
  void put(std::string key, T& value) {
    _writer->Key(key.c_str());
    _value(value);
  }
  virtual void serialize_impl() = 0;

 private:
  template <class T>
  void _value(T& value) {
    // should be an well-formated json object
    std::string raw_json = value.serialize();
    _writer->RawValue(raw_json.c_str(), raw_json.length(),
                      rapidjson::kObjectType);
  }
  void _value(std::string& value);
  template <class V>
  void _value(std::vector<V>& _vec);
  template <class V>
  void _value(std::map<std::string, V>& _map);

  rapidjson::StringBuffer* _buffer;
  rapidjson::Writer<rapidjson::StringBuffer>* _writer;
};

template <>
void JsonSerializer::_value<unsigned>(unsigned& value) {
  _writer->Uint(value);
}
template <>
void JsonSerializer::_value<int>(int& value) {
  _writer->Int(value);
}
template <>
void JsonSerializer::_value<int64_t>(int64_t& value) {
  _writer->Int64(value);
}
template <>
void JsonSerializer::_value<uint64_t>(uint64_t& value) {
  _writer->Uint64(value);
}
template <>
void JsonSerializer::_value<bool>(bool& value) {
  _writer->Bool(value);
}
template <>
void JsonSerializer::_value<double>(double& value) {
  _writer->Double(value);
}
template <>
void JsonSerializer::_value<float>(float& value) {
  _writer->Double(value);
}

void JsonSerializer::_value(std::string& value) {
  _writer->String(value.c_str(), value.length());
}
template <class V>
void JsonSerializer::_value(std::vector<V>& _vec) {
  _writer->StartArray();
  for (V& value : _vec) {
    _value(value);
  }
  _writer->EndArray(_vec.size());
}

template <class T>
void JsonSerializer::_value(std::map<std::string, T>& _map) {
  _writer->StartObject();
  for (auto& [key, value] : _map) {
    _writer->Key(key.c_str());
    _value(value);
  }
  _writer->EndObject();
}
}  // namespace hvs