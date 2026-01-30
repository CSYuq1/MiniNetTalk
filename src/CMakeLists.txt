
add_executable(connect_example ../example/daytime-client.cpp)
add_executable(http_exmple ../example/http_example.cpp)
add_executable(server server/server.cpp
        server/listener.cpp
        server/websocket_session.cpp
)