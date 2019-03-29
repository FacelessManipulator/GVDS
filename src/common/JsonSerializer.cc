#include "common/JsonSerializer.h"

using namespace hvs;

template <>
void JsonSerializer::encode<unsigned>(unsigned& value) {
    _writer->Uint(value);
}
template <>
void JsonSerializer::encode<int>(int& value) {
    _writer->Int(value);
}
template <>
void JsonSerializer::encode<int64_t>(int64_t& value) {
    _writer->Int64(value);
}
template <>
void JsonSerializer::encode<uint64_t>(uint64_t& value) {
    _writer->Uint64(value);
}
template <>
void JsonSerializer::encode<bool>(bool& value) {
    _writer->Bool(value);
}
template <>
void JsonSerializer::encode<double>(double& value) {
    _writer->Double(value);
}
template <>
void JsonSerializer::encode<float>(float& value) {
    _writer->Double(value);
}

void JsonSerializer::encode(std::string& _str) {
    _writer->String(_str.c_str(), _str.length());
}

template <>
int JsonSerializer::decode<unsigned>(rapidjson::Value* value, unsigned& dest) {
    if (!value->IsUint()) return -ERROR_INCORRECT_TYPE;
    dest = value->GetUint();
    return SUCCESS;
}

template <>
int JsonSerializer::decode<int>(rapidjson::Value* value, int& dest) {
    HANDLE_PARSE_ERR(!value->IsInt());
    dest = value->GetInt();
}

template <>
int JsonSerializer::decode<int64_t>(rapidjson::Value* value, int64_t& dest) {
    HANDLE_PARSE_ERR(!value->IsInt64());
    dest = value->GetInt64();
}

template <>
int JsonSerializer::decode<uint64_t>(rapidjson::Value* value, uint64_t& dest) {
    HANDLE_PARSE_ERR(!value->IsUint64());
    dest = value->GetUint64();
}

template <>
int JsonSerializer::decode<bool>(rapidjson::Value* value, bool& dest) {
    HANDLE_PARSE_ERR(!value->IsBool());
    dest = value->GetBool();
}

template <>
int JsonSerializer::decode<double>(rapidjson::Value* value, double& dest) {
    HANDLE_PARSE_ERR(!value->IsDouble());
    dest = value->GetDouble();
}

template <>
int JsonSerializer::decode<float>(rapidjson::Value* value, float& dest) {
    HANDLE_PARSE_ERR(!value->IsDouble());
    dest = (float)(value->GetDouble());
}

int JsonSerializer::decode(rapidjson::Value* value, std::string& _str) {
    HANDLE_PARSE_ERR(!value->IsString());
    _str.assign(value->GetString());
}

