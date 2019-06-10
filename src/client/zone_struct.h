//
// Created by yaowen on 5/28/19.
// 北航系统结构所-存储组
//

#ifndef HVSONE_ZONE_STRUCT_H
#define HVSONE_ZONE_STRUCT_H

#include "common/JsonSerializer.h"

namespace hvs{

    class GetZoneInfoRes : public hvs::JsonSerializer//区域信息返回
    {
    public:
        std::vector<std::string> zoneInfoResult;

    public:
        void serialize_impl() override {
            put("ZoneInfo", zoneInfoResult);
        };

        void deserialize_impl() override {
            get("ZoneInfo", zoneInfoResult);
        };

    public:
        GetZoneInfoRes() = default;
    };

    class SpaceInfo : public hvs::JsonSerializer//空间基本信息,考虑要不要map
    {
    public:
        std::vector<std::string> spaceID;//空间ID
        std::map<std::string, std::string> spaceName;//空间名
        std::map<std::string, int64_t> spaceSize;//空间容量

    public:
        void serialize_impl() override {
            put("UUID", spaceID);
            put("name", spaceName);
            put("capcity", spaceSize);
        };

        void deserialize_impl() override{
            get("UUID", spaceID);
            get("name", spaceName);
            get("capcity", spaceSize);
        };

    public:
        SpaceInfo() = default;
    };

    class ZoneInfo : public hvs::JsonSerializer//区域信息
    {
    public:
        std::string zoneID;//区域ID
        std::string zoneName;//区域名
        std::string ownerID;//区域主人账户ID
        std::vector<std::string> memberID;//区域成员账户ID
        SpaceInfo spaceBicInfo;//空间基本信息

    public:
        void serialize_impl() override {
            put("UUID", zoneID);
            put("name", zoneName);
            put("owner", ownerID);
            put("members", memberID);
            put("spaceinfo", spaceBicInfo);
        };

        void deserialize_impl() override{
            get("UUID", zoneID);
            get("name", zoneName);
            get("owner", ownerID);
            get("members", memberID);
            get("spaceinfo", spaceBicInfo);
        };

    public:
        ZoneInfo() = default;
    };
};
#endif //HVSONE_ZONE_STRUCT_H
