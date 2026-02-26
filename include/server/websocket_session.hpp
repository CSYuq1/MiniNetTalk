//
// Created by yuqi on 2026/2/4.
//
#include <memory>
#include <string>
#include <vector>

#include "server/shared_state.hpp"
#include "server/common.hpp"
#pragma once

#ifndef MININETTALK_WEBSOCKET_SESSION_HPP
#define MININETTALK_WEBSOCKET_SESSION_HPP

namespace minitalk::server {
        class websocket_session : std::enable_shared_from_this<websocket_session> {
                beast::flat_buffer                              buffer_;
                websocket::stream<beast::tcp_stream>            ws_; //beast::tcp_stream 最常用 还有支持ssl的stream底层
                std::shared_ptr<shared_state>                   state_;
                std::vector<std::shared_ptr<std::string const>> queue_; //写入的消息队列，一个用户一个websocket，一个队列
                //IO 回调层 集中放在这里-------------------------------------->
                static void fail(error_code ec, const std::string what);
                void        on_accept(error_code ec);
                void        on_read(error_code ec, std::size_t bytes_transferred);
                void        on_write(error_code ec, std::size_t bytes_transferred);
                //---------------------------------------------------------->

        public:
                websocket_session(
                        tcp::socket&&                        socket, //move 构造，转移生命权
                        std::shared_ptr<shared_state> const& state);
                ~websocket_session();

                template <class Body, class Allocator>
                void run(http::request<Body, http::basic_fields<Allocator>> req) {
                        // 设计模式名称：Option Pattern
                        //通过返回一个建议对象，增加可读性

                        ws_.set_option(
                                       //stream_base stream 的基础配置和类型容器,
                                       //组织类型，避免 namespace 污染
                                       websocket::stream_base::timeout::suggested(beast::role_type::server)
                                      );
                        auto opt = [](websocket::response_type& res) {
                                res.set(http::field::server,
                                        std::string(BOOST_BEAST_VERSION_STRING) +
                                        " websocket-chat-multi");
                        };
                        ws_.set_option(websocket::stream_base::decorator(opt));

                        /* beast为什么设计统一的接口websocket::asnyc_accept
                        * async_accept(req,res,handler) 确实更像“HTTP语义统一”，但代价是 API 语义会变得不清晰
                        库是否覆盖 res 的关键字段？（必须覆盖才能保证协议正确）
                        覆盖了又让用户觉得“我传 res 干嘛？”
                        不覆盖又会导致大量错误用法和兼容性问题
                        所以 Beast 选择：
                        响应由库生成（保证正确）
                        通过 decorator 给你“可控的定制窗口”
                        这既是安全性/正确性，也是可维护性/清晰语义的选择 🙂✨
                         */
                        ws_.async_accept(req,
                                         beast::bind_front_handler(&websocket_session::on_accept,
                                                                   shared_from_this()));
                }

                void send(std::shared_ptr<std::string const> const& msg);

        private: //内部实现层------------------------------------------------>
                void on_send(std::shared_ptr<std::string const> const& msg);
                //--------------------------------------------------------->
        };
} // minitalk::server

#endif //MININETTALK_WEBSOCKET_SESSION_HPP
