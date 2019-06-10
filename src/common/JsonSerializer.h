#pragma once

#include <hvsdef.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <map>
#include <string>
#include <vector>
#include <memory>

#define ERROR_INCORRECT_TYPE 111

// Currently we simply return the error code
// rather than handle the parse error
#define HANDLE_PARSE_ERR(ERR)              \
  do {                                     \
    if (ERR) return -ERROR_INCORRECT_TYPE; \
  } while (0);

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

  void deserialize(const char* jsonString) {
    rapidjson::Document document;
    document.Parse(jsonString);
    assert(document.IsObject());
    setJsonValue(document.GetObject());
    deserialize_impl();
  }

  void deserialize(const std::string& jsonString) {
    rapidjson::Document document;
    document.Parse(jsonString.c_str());
    assert(document.IsObject());
    setJsonValue(document.GetObject());
    deserialize_impl();
  }

  void deserialize(rapidjson::Value* value) {
    assert(value->IsObject());
    setJsonValue(value->GetObject());
    deserialize_impl();
  }

 public:
  JsonSerializer() {
    _buffer = new rapidjson::StringBuffer();
    _writer = new rapidjson::Writer<rapidjson::StringBuffer>(*_buffer);
  }
  JsonSerializer(JsonSerializer& oths) {
    // there is no need to support json value copying
    // _jsonValue.CopyFrom(oths._jsonValue, ALLOCATOR);
  }
  JsonSerializer(JsonSerializer&& oths) {}
  ~JsonSerializer() {
    delete _writer;
    delete _buffer;
  }

  void clear() {
    delete _writer;
    delete _buffer;
    _buffer = new rapidjson::StringBuffer();
    _writer = new rapidjson::Writer<rapidjson::StringBuffer>(*_buffer);
  }

 protected:
  template <class T>
  void put(std::string key, T& value) {
    _writer->Key(key.c_str());
    encode(value);
  }
  template <class T>
  bool get(std::string key, T& dest) {
    if (_jsonValue.HasMember(key.c_str())) {
      // the value would automaticly use move assignment
      decode(&(_jsonValue[key.c_str()]), dest);
      return true;
    }
    return false;
  }

  virtual void serialize_impl(){};
  virtual void deserialize_impl(){};

 protected:
  template <class T>
  void encode(T& value);
  void encode(std::string& _str);
  template <class V>
  void encode(std::vector<V>& _vec);
  template <class V>
  void encode(std::map<std::string, V>& _map);
  template <class V>
  void encode(std::shared_ptr<V> _ptr);

  template <class T>
  int decode(rapidjson::Value* value, T& dest);
  int decode(rapidjson::Value* value, std::string& _str);
  template <class V>
  int decode(rapidjson::Value* value, std::vector<V>& _vec);
  template <class V>
  int decode(rapidjson::Value* value, std::map<std::string, V>& _map);

  inline void setJsonValue(rapidjson::Value value) { this->_jsonValue = value; }

 protected:
  rapidjson::StringBuffer* _buffer;
  rapidjson::Writer<rapidjson::StringBuffer>* _writer;
  rapidjson::Value _jsonValue;
};

template <class V>
void JsonSerializer::encode(std::vector<V>& _vec) {
  _writer->StartArray();
  for (V& value : _vec) {
    encode(value);
  }
  _writer->EndArray(_vec.size());
}

template <class T>
void JsonSerializer::encode(std::map<std::string, T>& _map) {
  _writer->StartObject();
  for (auto& [key, value] : _map) {
    _writer->Key(key.c_str());
    encode(value);
  }
  _writer->EndObject();
}

template <class V>
void JsonSerializer::encode(std::shared_ptr<V> _ptr) {
  if(_ptr)
    encode<V>(*_ptr);
  else {
    int tmp = 0;
    encode<int>(tmp);
  }
}

template <>
void JsonSerializer::encode<unsigned>(unsigned& value);
template <>
void JsonSerializer::encode<int>(int& value);
template <>
void JsonSerializer::encode<int64_t>(int64_t& value);
template <>
void JsonSerializer::encode<uint64_t>(uint64_t& value);
template <>
void JsonSerializer::encode<bool>(bool& value);
template <>
void JsonSerializer::encode<double>(double& value);
template <>
void JsonSerializer::encode<float>(float& value);
template <class T>
void JsonSerializer::encode(T& value) {
  // should be an well-formated json object
  std::string raw_json = value.serialize();
  _writer->RawValue(raw_json.c_str(), raw_json.length(),
                    rapidjson::kObjectType);
}

template <class V>
int JsonSerializer::decode(rapidjson::Value* value, std::vector<V>& _vec) {
  HANDLE_PARSE_ERR(!value->IsArray());
  auto _arr = value->GetArray();
  std::vector<V> tmp(_arr.Size());
  for (int i = 0; i < _arr.Size(); i++) {
    decode(&(_arr[i]), tmp[i]);
  }
  _vec.swap(tmp);
}

template <class V>
int JsonSerializer::decode(rapidjson::Value* value,
                           std::map<std::string, V>& _map) {
  HANDLE_PARSE_ERR(!value->IsObject());
  auto _obj = value->GetObject();
  std::map<std::string, V> tmp;
  for (auto itr = value->MemberBegin(); itr != value->MemberEnd(); ++itr) {
    // the name of itr must be string
    decode(&(itr->value), tmp[itr->name.GetString()]);
  }
  _map.swap(tmp);
}

template <>
int JsonSerializer::decode<unsigned>(rapidjson::Value* value, unsigned& dest);
template <>
int JsonSerializer::decode<int>(rapidjson::Value* value, int& dest);
template <>
int JsonSerializer::decode<int64_t>(rapidjson::Value* value, int64_t& dest);
template <>
int JsonSerializer::decode<uint64_t>(rapidjson::Value* value, uint64_t& dest);
template <>
int JsonSerializer::decode<bool>(rapidjson::Value* value, bool& dest);
template <>
int JsonSerializer::decode<double>(rapidjson::Value* value, double& dest);
template <>
int JsonSerializer::decode<float>(rapidjson::Value* value, float& dest);
template <class T>
int JsonSerializer::decode(rapidjson::Value* value, T& dest) {
  dest.deserialize(value);
}
}  // namespace hvs