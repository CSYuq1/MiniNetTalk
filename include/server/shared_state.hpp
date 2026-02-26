//
// Created by yuqi on 2026/2/1.
//

#ifndef MININETTALK_SHARE_STATE_HPP
#define MININETTALK_SHARE_STATE_HPP

#include <boost/asio.hpp>
#include<boost/beast.hpp>
#include<unordered_set>

#include "server/common.hpp"
#include "server/config.hpp"


namespace minitalk::server {
        class websocket_session;

        class shared_state : std::enable_shared_from_this<shared_state> {
            std::string const doc_root_;

            // This mutex synchronizes all access to sessions_
            std::mutex mutex_;

            // Keep a list of all the connected clients
            std::unordered_set<websocket_session*> sessions_;

            //minitalk::server::config config_;

        public:
            explicit
            shared_state(std::string doc_root);

            std::string const&
            doc_root() const noexcept {
                return doc_root_;
            }

            void join(websocket_session* session);
            void leave(websocket_session* session);
            void send(std::string message);
        };
    }



#endif //MININETTALK_SHARE_STATE_HPP
