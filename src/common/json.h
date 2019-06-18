#pragma once

#include <mutex>
#include "JsonSerializer.h"

namespace hvs {

class __JSON_DENC : public JsonSerializer {
 public:
  template <class T>
  std::string json_encode(T& value) {
    // we need a mutex to protect the denc process
    std::lock_guard<std::mutex> denc_guard(_json_denc_mutex);
    return json_encode__nolock(value);
  }

  template <class T>
  void json_decode(const std::string& json_string, T& dest) {
    // we need a mutex to protect the denc process
    std::lock_guard<std::mutex> denc_guard(_json_denc_mutex);
    json_decode__nolock(json_string, dest);
  }

  __JSON_DENC() = default;

 private:
  template <class T>
  std::string json_encode__nolock(T& value) {
    // clear buffer first
    _buffer = new rapidjson::StringBuffer();
    _writer = new rapidjson::Writer<rapidjson::StringBuffer>(*_buffer);
    encode(value);
    // _buffer.GetString return an const char* buf pointer with zero copy
    // we need to copy it to string explicitly in case of the destroy of buf
    // area
    std::string res = _buffer->GetString();
    clear();
    return res;
  }

  template <class T>
  void json_decode__nolock(const std::string& json_string, T& dest) {
    rapidjson::Document document;
    rapidjson::Value value;
    document.Parse(json_string.c_str());
    value.Swap(document);
    decode(&value, dest);
    // _buffer.GetString return an const char* buf pointer with zero copy
    // we need to copy it to string explicitly in case of the destroy of buf
    // area
    // return std::string(_buffer->GetString());
  }

 private:
  std::mutex _json_denc_mutex;
};

// we don't have to return the pointer or rvalue ref. RVO works well.
// json_encode should only be used if the value is not a standard json string
// such as "[1,2,3]" or "1.31254". This function is slower than serialize.
template <class T>
std::string json_encode(T& value) {
  __JSON_DENC encoder;
  return encoder.json_encode(value);
}

// json_decode is slower than deserialize
template <class T>
void json_decode(const std::string& json_string, T& dest) {
  __JSON_DENC decoder;
  return decoder.json_decode(json_string, dest);
}
}  // namespace hvs
