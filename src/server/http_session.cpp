//
// Created by yuqi on 2026/2/2.
//

#include "http_session.hpp"

minitalk::server::http_session::http_session(
    tcp::socket&&                        socket,
    std::shared_ptr<shared_state> const& state)
    : stream_(std::move(socket)), //stream 包含了socket，在这初始化
      state_(state) {
}


void minitalk::server::http_session::do_read() {
    //初始化 parser，这是optional 的api
    parser_.emplace();

    //限制body长度
    parser_->body_limit(minitalk::server::config::default_body_limit);

    //设置超时,防止服务器资源耗尽
    stream_.expires_after(std::chrono::seconds(config::default_timeout));

    http::async_read(stream_, buffer_, *parser_, beast::bind_front_handler(&http_session::on_read,shared_from_this()));
}

void minitalk::server::http_session::fail(beast::error_code ec, char const* what) {
}

void minitalk::server::http_session::on_read(beast::error_code ec, std::size_t) {
}

void minitalk::server::http_session::on_write(beast::error_code ec, std::size_t, bool close) {
}

void minitalk::server::http_session::run() {
    do_read();
}

