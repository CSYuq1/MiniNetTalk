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

    void websocket_session::on_read(error_code ec,std::size_t bytes_transferred) {
            if (ec) [[unlikely]]
                fail(ec, "read");

            state_->send(beast::)

    }
}
