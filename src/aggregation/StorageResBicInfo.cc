#include "StorageResBicInfo.h"

using namespace hvs;

StorageResBicInfo::StorageResBicInfo()
{

}

StorageResBicInfo::StorageResBicInfo(std::string src_id, std::string src_name, std::string center_id,
std::string center_name, int64_t capacity, std::string  m_address, StorageResState res_state)
{
    storage_src_id = src_id;
    storage_src_name = src_name;
    host_center_id = center_id;
    host_center_name = center_name;
    total_capacity = capacity ;
    mgs_address = m_address;
    state = res_state;
}

void StorageResBicInfo::serialize_impl()
{
    put("storage_src_id", storage_src_id);
    put("storage_src_name", storage_src_name);
    put("host_center_id", host_center_id);
    put("host_center_name", host_center_name);
    put("total_capacity", total_capacity);
    put("mgs_address", mgs_address);
    put("state", state);
}

void StorageResBicInfo::deserialize_impl()
{
    get("storage_src_id", storage_src_id);
    get("storage_src_name", storage_src_name);
    get("host_center_id", host_center_id);
    get("host_center_name", host_center_name);
    get("total_capacity", total_capacity);
    get("mgs_address", mgs_address);
    get("state", state);
}

StorageResBicInfo::~StorageResBicInfo()
{

}
