//
// Created by yuqi on 2026/1/31.
//

#ifndef MININETTALK_LISTENER_HPP
#define MININETTALK_LISTENER_HPP
#include <memory>
#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include"common.hpp"//set "using namespace",uisng ...

namespace minitalk {
    namespace server {
        class shared_state;

        class listener : public std::enable_shared_from_this<listener> {//继承这个是为了捕获自己，用于异步回调函
            asio::io_context&            ioc_;
            asio::ip::tcp::acceptor      acceptor_;
            error_code                   ec_;
            std::shared_ptr<shared_state> state_;

        private:
            enum class listener_error_type {
                open,
                bind,
                set_option,
                listen,
                accept

            };

            int fail(error_code ec, listener_error_type);
            void keep_accept(error_code ec, tcp::socket);

        public:
            listener(asio::io_context&                   ioc,
                     asio::ip::tcp::endpoint            ep,
                     std::shared_ptr<shared_state> const& state);
            void run();
        };
    }
}
#endif //MININETTALK_LISTENER_HPP
