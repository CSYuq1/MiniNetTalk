//
// Created by yuqi on 2026/2/1.
//

#include "server/shared_state.hpp"
#include "server/websocket_session.hpp"

minitalk::server::shared_state::shared_state(std::string doc_root) : doc_root_(std::move(doc_root)) {
}

void minitalk::server::shared_state::join(websocket_session* session) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(session);
}

void minitalk::server::shared_state::leave(websocket_session* session) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(session);
}

void minitalk::server::shared_state::send(std::string message) {
    //减少拷贝，每个session 持有一个msg
    auto const msg = std::make_shared<std::string>(message);

    //最小锁原则 + 生命周期解耦 + 快照遍历（snapshot iteration）
    std::vector<std::weak_ptr<websocket_session>> v;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        v.reserve(sessions_.size());
        for (auto p : sessions_)
            v.emplace_back(p->weak_from_this());
    }

    for (auto const& wp : v)
        if (auto sp = wp.lock())
            sp->send(msg);
}
