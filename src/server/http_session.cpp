//
// Created by yuqi on 2026/2/2.
//

#include "http_session.hpp"

#include <iostream>

#include "websocket_session.hpp"

/**
 *
 * @tparam Body
 * @tparam Allocator
 * @param doc_root  url
 */
template <class Body, class Allocator>
http::message_generator
handle_request(
    beast::string_view                                   doc_root,
    http::request<Body, http::basic_fields<Allocator>>&& req) {

}

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
    parser_->body_limit(config::default_body_limit);

    //设置超时,防止服务器资源耗尽
    stream_.expires_after(std::chrono::seconds(config::default_timeout));

    http::async_read(stream_, buffer_, *parser_, beast::bind_front_handler(&http_session::on_read, shared_from_this()));
}

void minitalk::server::http_session::fail(beast::error_code ec, char const* what) {
    if (ec == asio::error::operation_aborted) { return; }
    std::cerr << what << ": " << ec.message() << std::endl;
}

/**
 * @brief 如果有连接请求优先升级，否则正常响应http
 *
 */
void minitalk::server::http_session::on_read(beast::error_code ec, std::size_t) {
    // 这意味着他们关闭了连接
    if (ec == http::error::end_of_stream) {
        //相当于四次挥手的第一次
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
        return;
    }

    // 处理错误（如果有）
    if (ec)
        return fail(ec, "read");

    // 检查是否是 WebSocket 升级请求
    if (websocket::is_upgrade(parser_->get())) {
        // 创建 websocket 会话，转移所有权
        // 包括 socket 和 HTTP 请求。
        boost::make_shared<websocket_session>(stream_.release_socket(), state_)->run(parser_->release());
        return;
    }



    http::message_generator msg =handle_request(state_->doc_root(), parser_->release());

    // 确定是否应该关闭连接
    bool keep_alive = msg.keep_alive();

    auto self = shared_from_this();

    // 发送响应
    beast::async_write(
                       stream_, std::move(msg),
                       [self, keep_alive](beast::error_code ec, std::size_t bytes) {
                           self->on_write(ec, bytes, keep_alive);
                       });
}

void minitalk::server::http_session::on_write(beast::error_code ec, std::size_t, bool keep_alive) {
    // 处理错误（如果有）
    if (ec)
        return fail(ec, "write");

    if (!keep_alive) {
        //这个和头文件不同
        // 这意味着我们应该关闭连接，通常是因为
        // 响应指示了 "Connection: close" 语义。
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
        return;
    }

    // 读取另一个请求
    do_read();
}

void minitalk::server::http_session::run() {
    do_read();
}

