//
// Created by yuqi on 2026/1/31.
//

#ifndef MININETTALK_COMMON_HPP
#define MININETTALK_COMMON_HPP
//set namespace of boost lib
using tcp        = boost::asio::ip::tcp;
using udp        = boost::asio::ip::udp;
using error_code = boost::system::error_code;

namespace asio = boost::asio;
namespace beast = boost::beast;// from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;




#endif //MININETTALK_COMMON_HPP
