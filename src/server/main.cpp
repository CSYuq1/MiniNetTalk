//
// Created by yuqi on 2026/1/24.
//
#include<cstdint>
#include<print>
#include<filesystem>
#include"boost/asio.hpp"
#include"boost/beast.hpp"

#include"common.hpp"//set "using namespace",uisng ...
#include "listener.hpp"
#include"shared_state.hpp"
#pragma once


namespace minitalk {
	namespace server {
		constexpr uint16_t    listen_port    = 49999;
		constexpr std::string listen_address = "0.0.0.0";
		constexpr std::string doc_root_path  = "minitalk";
		uint16_t              num_threads    = std::thread::hardware_concurrency(); //妈的，不会有人用255个核心以上的服务器跑我写的程序吧？
		struct config {
			uint_least16_t        listen_port_;
			uint32_t              address_int_;
			std::string_view      address_;
			std::filesystem::path doc_root_path_;
			uint16_t              thread_count_;

			asio::ip::port_type

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
				  address_(address_str),
				  doc_root_path_(doc_root_path),
				  thread_count_(thread_count) {
			}


			//config(const std::filesystem::path& config_root_path) {
			//}

			/**
			 * 不指定参数的默认初始化
			 */
			config() {
				listen_port_   = listen_port;
				address_int_   = std::stoul(listen_address);
				address_       = listen_address;
				doc_root_path_ = doc_root_path;
				thread_count_  = std::thread::hardware_concurrency();
			}
		};
	}
}

std::optional<minitalk::server::config> config;


int main(int argc, char* argv[]) noexcept {
	if (argc == 5) {
		config = minitalk::server::config(std::stoi(argv[1]),
		                                  argv[2],
		                                  argv[3],
		                                  std::stoi(argc[4])
		                                 );
	}
	else {
		config = minitalk::server::config();
	}
	asio::io_context ioc;
	// Create and launch a listening port
	boost::make_shared<minitalk::server::listener>(
	                                               ioc,
	                                               tcp::endpoint{config->address_,},
	                                               boost::make_shared<shared_state>(doc_root))->run();


	return 0;
}
