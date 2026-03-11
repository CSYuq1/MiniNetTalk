//
// Created by yuqi on 2026/2/4.
//

#include "server/websocket_session.hpp"

#include <iostream>

#include"server/shared_state.hpp"

namespace minitalk::server {
    websocket_session::websocket_session(tcp::socket&&                        socket,
                                         std::shared_ptr<shared_state> const& state) :
        ws_(std::move(socket)),
        state_(state) {
    }

    websocket_session::~websocket_session() {
        state_->leave(this);
    }

    void websocket_session::fail(error_code ec, const std::string what) {
        if (ec == asio::error::operation_aborted || ec == websocket::error::closed)
            return;
        std::cerr << what << ": " << ec.message() << "\n";
    }

    void websocket_session::on_accept(error_code ec) {
        if (ec) [[unlikely]] {
            return fail(ec, "accept");
        }
        //添加到会话中
        state_->join(this);

        ws_.async_read(buffer_,
                       beast::bind_front_handler(&websocket_session::on_read,
                                                 shared_from_this()));
    }

    void websocket_session::on_read(error_code ec, [[maybe_unused]] std::size_t bytes_transferred) {
        if (ec) [[unlikely]]{
            return fail(ec, "read");
        }

        //debug 测试
        auto msg = beast::buffers_to_string(buffer_.data());

        std::cout << "[RECV] from session " << this
            << " : " << msg << "\n";

        // send , on_send 会在里面被调用
        state_->send(beast::buffers_to_string(buffer_.data()));
        // consume all buffer ->clear buffer
        buffer_.consume(buffer_.size());

        //继续读信息
        ws_.async_read(buffer_, beast::bind_front_handler(
                                                          &websocket_session::on_read,
                                                          shared_from_this()));
    }

    void websocket_session::send(std::shared_ptr<const std::string> const& msg) {
        asio::post(ws_.get_executor(), //get ioc
                   beast::bind_front_handler(
                                             &websocket_session::on_send,
                                             shared_from_this(), msg
                                            ));
    }

    void websocket_session::on_send(std::shared_ptr<std::string const> const& msg) {
        //防止服务器被刷爆
        if (queue_.size() >= 1024) {
            // 丢弃/断开都行，至少别无限堆积
            return;
        }

        queue_.push_back(msg);

        //如果队列里已经有正在发送的消息，就什么都不做。
        if (queue_.size() > 1)
            return;

        ws_.async_write(
                        asio::buffer(*queue_.front()),
                        beast::bind_front_handler(
                                                  &websocket_session::on_write,
                                                  shared_from_this()));
    }

    void websocket_session::on_write(error_code ec, [[maybe_unused]] std::size_t bytes_transferred) {
        //处理write 过程出现的错误
        if (ec)
            return fail(ec, "write");
        //写完，就清除队列元素 vector 元素排列紧凑，可以运用缓存，
        queue_.erase(queue_.begin());

        // Send the next message if any
        if (!queue_.empty())
            ws_.async_write(
                            asio::buffer(*queue_.front()),
                            beast::bind_front_handler(
                                                      &websocket_session::on_write,
                                                      shared_from_this()));
    }
}
