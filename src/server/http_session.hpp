//
// Created by yuqi on 2026/2/2.
//

#ifndef MININETTALK_HTTP_SESSION_HPP
#define MININETTALK_HTTP_SESSION_HPP
#include <memory>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "share_state.hpp"
#include"common.hpp"//set "using namespace",uisng ...

namespace minitalk::server {
    class http_session : public std::enable_shared_from_this<http_session> {
        asio::io_service&               ioc_;
        beast::tcp_stream               stream_;
        beast::flat_buffer              buffer_;
        boost::shared_ptr<shared_state> state_;

    public:
        http_session(asio::io_context ioc);
    };
}


#endif //MININETTALK_HTTP_SESSION_HPP
