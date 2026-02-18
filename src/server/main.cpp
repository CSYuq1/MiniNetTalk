
//
// Created by yuqi on 2026/1/24.
//
#include<cstdint>
#include<print>
#include<filesystem>
#include <iostream>

#include"boost/asio.hpp"
#include"boost/beast.hpp"

#include"common.hpp"//set "using namespace"
#include "listener.hpp"
#include"shared_state.hpp"
#pragma once


namespace minitalk {
    namespace server {
        constexpr uint16_t    default_listen_port = 49999;
        constexpr std::string default_listen_address = "0.0.0.0";
        constexpr std::string default_doc_root_path = "minitalk";
        uint16_t              num_threads = std::thread::hardware_concurrency(); //妈的，不会有人用255个核心以上的服务器跑我写的程序吧？
        struct config {
            uint_least16_t        listen_port_;
            asio::ip::address     address_;
            uint32_t              address_int_;
            std::string           address_str_;
            std::filesystem::path doc_root_path_;
            uint16_t              thread_count_;


            /**
             *
             * @brief 默认构造，处理管理员传递的参数
             */
            config(uint16_t                     port,
                   const std::string&           address_str,
                   const std::filesystem::path& doc_root_path,
                   uint16_t                     thread_count)

                : listen_port_(port),
                  address_int_(std::stoul(address_str)),
                  address_str_(address_str),
                  doc_root_path_(doc_root_path),
                  thread_count_(thread_count),
                  address_(asio::ip::make_address(address_str)) {
            }


            //config(const std::filesystem::path& config_root_path) {
            //}

            /**
             * 不指定参数的默认初始化,仅限调试
             */
            config() :
                listen_port_(default_listen_port),
                address_(asio::ip::make_address(default_listen_address)),
                address_int_(std::stoul(default_listen_address)),
                address_str_(default_listen_address),
                doc_root_path_(default_doc_root_path) {
            }
        };
    }
}

std::optional<minitalk::server::config> config;


int main(int argc, char* argv[]) noexcept {
    if (argc == 5)[[unlikely]] {
        config = minitalk::server::config(std::stoi(argv[1]),
                                          argv[2],
                                          argv[3],
                                          std::stoi(argv[4])
                                         );
    }
    else {
        config = minitalk::server::config();
    }
    asio::io_context ioc;
    // Create and launch a listening port
    error_code ec;

    auto addr = asio::ip::make_address(config->address_str_, ec);
    if (ec) [[unlikely]] {
        std::cerr << ec.message() << "\n";
    }

    std::make_shared<minitalk::server::listener>(
                                                 ioc,
                                                 tcp::endpoint{config->address_, config->listen_port_},
                                                 std::make_shared<
                                                     minitalk::server::shared_state>(config->doc_root_path_))->run();


    return 0;
}
