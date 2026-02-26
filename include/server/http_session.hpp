//
// Created by yuqi on 2026/2/2.
//

#ifndef MININETTALK_HTTP_SESSION_HPP
#define MININETTALK_HTTP_SESSION_HPP
#include <memory>
#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "server/shared_state.hpp"
#include "server/common.hpp"
#include "server/config.hpp"

namespace minitalk::server {
    class http_session : public std::enable_shared_from_this<http_session> {
        beast::tcp_stream             stream_;
        beast::flat_buffer            buffer_;
        std::shared_ptr<shared_state> state_;
        // The parser is stored in an optional container so we can
        // construct it from scratch it at the beginning of each new message.
        //construct → use → destroy

        std::optional<http::request_parser<http::string_body>> parser_;

        struct send_lambda;

        void fail(beast::error_code ec, char const* what);
        void do_read();
        void on_read(beast::error_code ec, std::size_t);
        void on_write(beast::error_code ec, std::size_t, bool close);

    public:
        http_session(
            tcp::socket&&                        socket,
            std::shared_ptr<shared_state> const& state);

        void run();
    };
}


#endif //MININETTALK_HTTP_SESSION_HPP
