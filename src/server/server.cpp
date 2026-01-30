//
// Created by yuqi on 2026/1/24.
//
#include<print>
#include"boost/asio.hpp"
#include"boost/beast.hpp"


namespace asio = boost::asio;
namespace beast = boost::beast;

using tcp        = boost::asio::ip::tcp;
using udp        = boost::asio::ip::udp;
using error_code = boost::system::error_code;

namespace mini_talk
{
	namespace server
	{

	}
}

int main(int argc, char* argv[])
{
	asio::io_context io_context;
	error_code       ec;
	tcp::acceptor    acceptor(io_context, tcp::endpoint(tcp::v6(), mini_talk::server::listen_port));

	return 0;
}
