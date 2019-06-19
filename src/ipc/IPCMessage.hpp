/**
 * Created by Yaowen Xu on 2019-03-28.
 * 作者: Yaowen Xu
 * 时间: 2019-03-28
 * 工程: HVSONE
 * 作者单位: 北京航空航天大学计算机学院-系统结构研究所
 */

#ifndef HVSONE_IPCMESSAGE_H
#define HVSONE_IPCMESSAGE_H

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace hvs{
    class IPCMessage{
    public:
        enum { header_length = 4 };
        enum { max_body_length = 1048576 }; // 最大的消息长度为 512Byte, 可封装结构体；
        IPCMessage():body_length_(0){
        }

        const char* data() const
        {
            return data_;
        }

        char* data()
        {
            return data_;
        }


        std::size_t length() const
        {
            return header_length + body_length_;
        }

        const char* body() const
        {
            return data_ + header_length;
        }

        char* body()
        {
            return data_ + header_length;
        }

        std::size_t body_length() const
        {
            return body_length_;
        }

        void body_length(std::size_t new_length)
        {
            body_length_ = new_length;
            if (body_length_ > max_body_length){
                body_length_ = max_body_length;
                std::cerr << "错误：超出消息可存储最大值！" << std::endl;
            }
        }

        // 消息头解码操作
        bool decode_header()
        {
            char header[header_length + 1] = "";
            std::strncat(header, data_, header_length);
            body_length_ = std::atoi(header);
            if (body_length_ > max_body_length)
            {
                body_length_ = 0;
                return false;
            }
            return true;
        }

        // 消息头编码操作
        void encode_header()
        {
            char header[header_length + 1] = "";
            std::sprintf(header, "%4d", static_cast<int>(body_length_));
            std::memcpy(data_, header, header_length);
        }

        // utils 通过字符串创建一个新的消息；
        static std::shared_ptr<IPCMessage> /*IPCMessage*/ make_message_by_charstring(const char* str){
    //        进行声明智能指针，防止发生内存拷贝；
    //        IPCMessage msg;
    //        return msg;
            IPCMessage* msg = new IPCMessage();
            msg->body_length(std::strlen(str));
            std::memcpy(msg->body(), str, msg->body_length());
            msg->encode_header();
            return std::shared_ptr<IPCMessage>(msg);
        }

    private:
        char data_[header_length + max_body_length];
        std::size_t body_length_;
    };
}

#endif //HVSONE_IPCMESSAGE_H
