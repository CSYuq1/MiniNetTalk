#include<print>
#include"boost/asio.hpp"

using namespace boost;

int main(int argc, char* argv[])
{
    asio::io_context   io_context;
    system::error_code ec;

    asio::ip::tcp::resolver resolver(io_context);
    auto                    endpoints = resolver.resolve("time-c.timefreq.bldrdoc.gov", "13");
    if (!endpoints.empty())
    {
        std::println("get ip success!");
        uint32_t i = 1;
        for (auto& res : endpoints)
        {
            std::print(" ip{}: {}\n", i++, res.endpoint().address().to_string());
        }
    }

    asio::ip::tcp::socket socket{io_context};
    auto                  endpoint = asio::connect(socket, endpoints, ec);
    if (!ec)
        std::println("connect success!");


    asio::streambuf buffer{1024};
    asio::async_read(socket, buffer,
                     [&](const system::error_code& error_code, std::size_t bytes_transferred)
                     {
                         std::println("buffer.size() = {}", buffer.size());
                         std::println("ec = {}", error_code ? error_code.message() : "success");

                         std::string data(
                                          asio::buffers_begin(buffer.data()),
                                          asio::buffers_begin(buffer.data()) + bytes_transferred
                                         );
                         std::println("recv data: {} ", data);
                         buffer.consume(bytes_transferred);
                     });
    //std::print("hello");
    io_context.run();
    socket.close();
    return 0;
}
