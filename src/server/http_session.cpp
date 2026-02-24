//
// Created by yuqi on 2026/2/2.
//
#include <iostream>
#include "http_session.hpp"
#include "websocket_session.hpp"


/**
 * @tparam Body
 * @tparam Allocator http-header-fields 的底层内存模型
 * @param doc_root  url
 HTTP header fields container.
    ///
    /// This stores the HTTP header section of the request.
    ///
    /// HTTP message layout:
    ///
    ///   GET /index.html HTTP/1.1
    ///   Host: example.com
    ///   Connection: keep-alive
    ///   Content-Length: 5
    ///
    ///   Hello
    ///
    /// The lines above (Host, Connection, Content-Length)
    /// are stored in `http::basic_fields`.
    ///
    /// In Beast:
    ///
    ///   http::request
    ///     ├── basic_fields (header fields)
    ///     └── body
    ///
    /// Header fields describe metadata such as:
    ///   - content type
    ///   - content length
    ///   - connection behavior
    ///   - host
    ///
 */
template <class Body, class Allocator>
http::message_generator
handle_request(
    beast::string_view doc_root,

    http::request<Body, http::basic_fields<Allocator>>&& req) {
    // return  bad request
    auto const bad_req = [ver=req.version(),keep_alive=req.keep_alive()]
    (std::string_view why) {
        //http 头部startline 的部分是强制性设置，可以在构造函数中完成
        http::response<http::string_body> res{http::status::bad_request, ver};
        //方便调试的字段，可以不写
        res.set(http::field::server, "minitalk");
        //声明body的类型，让客户端可以解析
        res.set(http::field::content_type, "text/html");
        res.keep_alive(keep_alive);
        res.body() = "Bad request: ' " + std::string(why) + "'";
        //填写body 长度，必须在最后面调用
        res.prepare_payload();
        return res;
    };

    auto const not_found = [ver=req.version(),keep_alive=req.keep_alive()]
    (std::string_view target) {
        http::response<http::string_body> res{http::status::not_found, ver};
        res.set(http::field::server, "minitalk");
        res.set(http::field::content_type, "text/html");
        res.keep_alive(keep_alive);
        res.body() = "The' " + std::string(target) + "' not found!";
        res.prepare_payload();
        return res;
    };

    auto const server_error = [ver=req.version(),keep_alive=req.keep_alive()]
    (std::string_view why) {
        http::response<http::string_body> res{http::status::internal_server_error, ver};
        res.set(http::field::server, "minitalk");
        res.set(http::field::content_type, "text/html");
        res.keep_alive(keep_alive);
        res.body() = "Server error: '" + std::string(why) + "'";
        res.prepare_payload();
        return res;
    };

    //http::verb 是http::method 的口语化叫法
    if (req.method() != http::verb::get &&
        req.method() != http::verb::head)
        return bad_req("Unknown HTTP-method");

    // 请求路径必须是绝对的，且不能包含 ".."。
    //这相当于一个究极简化的安全检查,实际生产环境会包装为独立函数
    if (req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos)
        return bad_req("Illegal request-target");

    std::string path(path_catch(doc_root, req.target()));
    if (req)

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
    //parser_->get() 获取request的引用
    if (websocket::is_upgrade(parser_->get())) {
        // 创建 websocket 会话，转移所有权
        // 包括 socket 和 HTTP 请求。
        boost::make_shared<websocket_session>(stream_.release_socket(), state_)->run(parser_->release());
        return;
    }


    http::message_generator msg = handle_request(state_->doc_root(),
                                                 /*取出 request 对象，并清空 parser*/parser_->release());

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

