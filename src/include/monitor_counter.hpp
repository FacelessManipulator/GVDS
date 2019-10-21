#pragma once
#include "common/JsonSerializer.h"

namespace hvs{
    struct MonitorSpeed: public JsonSerializer {
        typedef std::pair<std::uint64_t, std::atomic_uint64_t> COUNTER;
        COUNTER byte_read_local;
        COUNTER byte_write_local;
        COUNTER byte_read_remote;
        COUNTER byte_write_remote;
        uint64_t byte_read_local_speed;
        uint64_t byte_write_local_speed;
        uint64_t byte_read_remote_speed;
        uint64_t byte_write_remote_speed;

        void flush_speed(uint32_t period_in_ms) {
            byte_read_local_speed = (byte_read_local.second - byte_read_local.first)*1000/period_in_ms;
            byte_write_local_speed = (byte_write_local.second - byte_write_local.first)*1000/period_in_ms;
            byte_read_remote_speed = (byte_read_remote.second - byte_read_remote.first)*1000/period_in_ms;
            byte_write_remote_speed = (byte_write_remote.second - byte_write_remote.first)*1000/period_in_ms;
            byte_read_local.first = byte_read_local.second;
            byte_write_local.first = byte_write_local.second;
            byte_read_remote.first = byte_read_remote.second;
            byte_write_remote.first = byte_write_remote.second;
        }

        MonitorSpeed& operator = (const MonitorSpeed& mso) {
            this->byte_read_local_speed = mso.byte_read_local_speed;
            this->byte_write_local_speed = mso.byte_write_local_speed;
            this->byte_read_remote_speed = mso.byte_read_remote_speed;
            this->byte_write_remote_speed = mso.byte_write_remote_speed;
        }

        void serialize_impl() override {
            // we do not apply lock cuz inconsistent monitor data also works
            put("brls", byte_read_local_speed);
            put("brrs", byte_read_remote_speed);
            put("bwls", byte_write_local_speed);
            put("bwrs", byte_write_remote_speed);
        }

        void deserialize_impl() override {
            get("brls", byte_read_local_speed);
            get("brrs", byte_read_remote_speed);
            get("bwls", byte_write_local_speed);
            get("bwrs", byte_write_remote_speed);
        }
    };
};