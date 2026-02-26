
//
// Created by yuqi on 2026/1/24.
//
#include<cstdint>
#include<print>
#include <filesystem>
#include <iostream>
#include <utility>

#include "boost/asio.hpp"
#include "boost/beast.hpp"

#include "server/common.hpp"
#include "server/listener.hpp"
#include "server/shared_state.hpp"
#include "server/config.hpp"
#pragma once


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
    error_code       ec;

    std::make_shared<minitalk::server::listener>(ioc,
                                                 tcp::endpoint(config->address_, config->listen_port_),
                                                 std::make_shared<
                                                     minitalk::server::shared_state>(config->doc_root_path_))->run();


    //暴力的终止程序, 还需要后期完善，不能保证持久化模块正常退出
    asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
                       [&ioc](boost::system::error_code const&, int) {
                           ioc.stop();
                       });

    std::vector<std::thread> v;
    //设置n-1 个线程,main本身也算一个线程
    v.reserve(config->thread_count_ - 1);
    for (auto i = config->thread_count_ - 1; i > 0; --i)
        v.emplace_back(
                       [&ioc] {
                           ioc.run();
                       });
    //不要忘记了主线程
    ioc.run();
    return 0;
}
