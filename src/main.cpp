#include<print>
#include"asio.hpp"

int main(int argc, char* argv[]) {
    asio::io_context io_context;
    asio::error_code ec;

    asio::ip::tcp::resolver    resolver(io_context);
    auto                       ip_res = resolver.resolve("bilibili.com", "http");
    asio::ip::detail::endpoint endpoint{asio::ip::make_address_v4("8.8.8.8", ec), 80};
    asio::ip::tcp::socket      socket{io_context};

    std::print("hello");
    return 0;
}
