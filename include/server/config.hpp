//
// Created by yuqi on 2026/2/19.
//

#ifndef MININETTALK_CONFIG_HPP
#define MININETTALK_CONFIG_HPP
#include <filesystem>

namespace minitalk::server {
    struct config {
        //constexpr:
        static constexpr std::size_t      default_body_limit     = 10000;
        static constexpr auto             default_timeout        = std::chrono::seconds(30);
        static constexpr uint16_t         default_listen_port    = 49999;
        static constexpr std::string_view default_listen_address = "0.0.0.0";
        static constexpr std::string_view default_doc_root_path  = "/home/yuqi/CLionProjects/MiniNetTalk/static";
        static constexpr std::string_view project_name           = "minitalk";

        //need  construct:
        uint_least16_t        listen_port_;
        asio::ip::address     address_;
        std::string           address_str_;
        std::filesystem::path doc_root_path_;
        uint16_t              thread_count_ = std::thread::hardware_concurrency();;
        // 默认构造，处理管理员传递的参数
        config(uint16_t              port,
               const std::string&    address_str,
               std::filesystem::path doc_root_path,
               uint16_t              thread_count) :
            listen_port_(port),
            address_str_(address_str),
            doc_root_path_(std::move(doc_root_path)),
            thread_count_(thread_count),
            address_(asio::ip::make_address(address_str)) {
        }


        // 不指定参数的默认初始化,仅限调试
        config() :
            listen_port_(default_listen_port),
            address_(asio::ip::make_address(default_listen_address)),
            address_str_(default_listen_address),
            doc_root_path_(default_doc_root_path) {
        }
    };
}
#endif //MININETTALK_CONFIG_HPP
