//
// Created by yuqi on 2026/2/2.
//
#include <iostream>
#include "server/http_session.hpp"
#include "server/websocket_session.hpp"

/**
 *  Multipurpose Internet Mail Extensions -> mime 文件的后缀
 * @brief 处理path后缀，返回正确的content_type
 * @param path 请求的路径
 * @return content_type
 */
std::string_view mime_type(std::string_view path) {
    using beast::iequals;

    auto             pos = path.rfind('.');
    std::string_view ext = (pos == beast::string_view::npos) //npos 是没有找到的意思
                               ? std::string_view{}
                               : path.substr(pos);
    // "start----pos----end" ->  "pos---end"只保留pos到结尾，相当于substr(pos,str.size()-1)

    struct mapping {
        std::string_view ext;
        std::string_view mime;
    };

    static constexpr mapping table[] = {
        {".htm", "text/html"},
        {".html", "text/html"},
        {".php", "text/html"},
        {".css", "text/css"},
        {".txt", "text/plain"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".xml", "application/xml"},
        {".swf", "application/x-shockwave-flash"},
        {".flv", "video/x-flv"},
        {".png", "image/png"},
        {".jpe", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".jpg", "image/jpeg"},
        {".gif", "image/gif"},
        {".bmp", "image/bmp"},
        {".ico", "image/vnd.microsoft.icon"},
        {".tiff", "image/tiff"},
        {".tif", "image/tiff"},
        {".svg", "image/svg+xml"},
        {".svgz", "image/svg+xml"},
    };

    for (auto const& m : table)
        if (iequals(ext, m.ext))
            return m.mime;

    return "application/octet-stream"; // 更通用的默认值
}

std::string
path_concat(
    std::string_view base,
    std::string_view target) {
    if (base.empty())[[unlikely]]
        return std::string(target);

    std::string res(base);

    //Windows
#ifdef BOOST_MSVC
    std::cout << "didnt write this func ";

    //Linux
#else
    //移除多余的‘/’ 假如不移除 --> /var/ + /www/images/logo.png == /var//www/images/logo.png
    if (res.back() == '/')
        res.pop_back();
    res.append(target);
#endif
    return res;
}

/**
 * @tparam Body
 * @tparam Allocator http-header-fields 的底层内存模型
 * @param doc_root  服务器储存http文件的根目录
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
        res.set(http::field::server, minitalk::server::config::project_name);
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
        res.set(http::field::server, minitalk::server::config::project_name);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(keep_alive);
        res.body() = "The' " + std::string(target) + "' not found!";
        res.prepare_payload();
        return res;
    };

    auto const server_error = [ver=req.version(),keep_alive=req.keep_alive()]
    (std::string_view why) {
        http::response<http::string_body> res{http::status::internal_server_error, ver};
        res.set(http::field::server, minitalk::server::config::project_name);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(keep_alive);
        res.body() = "Server error: '" + std::string(why) + "'";
        res.prepare_payload();
        return res;
    };

    if (req.target().empty() ||
        req.target()[0] != '/' ||
        //路径穿越漏洞 ，防止在上级读取敏感文件
        req.target().find("..") != std::string_view::npos)
        return bad_req("Illegal request-target");

    std::string path(path_concat(doc_root, req.target()));
    if (req.target().back() == '/')
        path.append("index.html");

    error_code                  ec;
    http::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);

    //处理文件目录不存在的问题
    if (ec == boost::system::errc::no_such_file_or_directory)[[unlikely]]{
    }

    if (ec) [[unlikely]]{
        return server_error(ec.message());
    }

    //**HTTP 的 HEAD 语义要求“返回与 GET 尽量一致的响应头（headers），但响应体（body）必须为空
    auto const head = [path=path,ver=req.version(),keep_alive=req.keep_alive(),body_size=body.size()]
    () {
        http::response<http::empty_body> res{http::status::ok, ver};
        res.set(http::field::server, minitalk::server::config::project_name);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(body_size);
        res.keep_alive(keep_alive);
        return res;
    };

    auto const get = [path=path,ver=req.version(),keep_alive=req.keep_alive(),&body,body_size=body.size()] {
        http::response<http::file_body> res{
            std::piecewise_construct, //<file_body>特有的一种构造方式
            std::make_tuple(std::move(body)),
            //通过传入构造参数而不是构造好的对象，使子对象在目标内存中直接构造，从而避免临时对象、避免拷贝和额外移动，同时支持 move-only 类型，并提供统一的泛型构造机制。
            std::make_tuple(http::status::ok, ver)
        };
        res.set(http::field::server, minitalk::server::config::project_name);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(body_size);
        res.keep_alive(keep_alive);
        return res;
    };

    switch (req.method()) {
    case http::verb::head:
        return head();
    case http::verb::get: [[likely]]
            return get();
    default: [[unlikely]]
            return bad_req("Unknown HTTP-method");
    }
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
    //parser read data from buf
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

