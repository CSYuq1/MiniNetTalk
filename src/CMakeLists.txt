# src/CMakeLists.txt

# ---- common include root ----
set(MININETTALK_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")

# ---- (optional but recommended) threads ----
find_package(Threads REQUIRED)

# ---- (optional) Boost::system ----
# Beast/Asio 基本是 header-only，但有些环境 Boost.System 需要链接库
option(MININETTALK_LINK_BOOST_SYSTEM "Link Boost::system if your toolchain requires it" OFF)
if(MININETTALK_LINK_BOOST_SYSTEM)
    find_package(Boost REQUIRED COMPONENTS system)
else()
    find_package(Boost REQUIRED)
endif()

# ============================================================
# server executable
# ============================================================
add_executable(server
        server/main.cpp
        server/listener.cpp
        server/config_reader.cpp
        server/shared_state.cpp
        server/http_session.cpp
        server/websocket_session.cpp
)

target_include_directories(server
        PRIVATE
        $<BUILD_INTERFACE:${MININETTALK_INCLUDE_DIR}>
)

target_link_libraries(server
        PRIVATE
        Threads::Threads
)
#target_compile_options(server PRIVATE -fsanitize=address -fno-omit-frame-pointer)
#target_link_options(server PRIVATE -fsanitize=address)
if(MININETTALK_LINK_BOOST_SYSTEM)
    target_link_libraries(server PRIVATE Boost::system)
endif()

# ============================================================
# Optional examples (only if files exist)
# If you don't want examples here, you can delete this section.
# ============================================================
if(EXISTS "${PROJECT_SOURCE_DIR}/example/daytime-client.cpp")
    add_executable(connect_example
            ../example/daytime-client.cpp
    )
    target_include_directories(connect_example PRIVATE
            $<BUILD_INTERFACE:${MININETTALK_INCLUDE_DIR}>
    )
    target_link_libraries(connect_example PRIVATE Threads::Threads)
    if(MININETTALK_LINK_BOOST_SYSTEM)
        target_link_libraries(connect_example PRIVATE Boost::system)
    endif()
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/example/http_example.cpp")
    add_executable(http_example
            ../example/http_example.cpp
    )
    target_include_directories(http_example PRIVATE
            $<BUILD_INTERFACE:${MININETTALK_INCLUDE_DIR}>
    )
    target_link_libraries(http_example PRIVATE Threads::Threads)
    if(MININETTALK_LINK_BOOST_SYSTEM)
        target_link_libraries(http_example PRIVATE Boost::system)
    endif()
endif()