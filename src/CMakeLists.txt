
add_executable(connect_example ../example/daytime-client.cpp
        server/listener.cpp
        server/listener.hpp)
add_executable(http_exmple ../example/http_example.cpp)
add_executable(server
        server/main.cpp
        server/listener.cpp
        server/websocket_session.cpp
        server/config_reader.cpp
        server/common.hpp
        server/shared_state.cpp
        server/shared_state.hpp
        server/http_session.cpp
        server/http_session.hpp
)