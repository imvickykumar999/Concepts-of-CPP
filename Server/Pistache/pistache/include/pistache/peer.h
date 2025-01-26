/*
 * SPDX-FileCopyrightText: 2015 Mathieu Stefani
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* peer.h
   Mathieu Stefani, 12 August 2015

  A class representing a TCP Peer
*/

#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <pistache/async.h>
#include <pistache/http.h>
#include <pistache/net.h>
#include <pistache/os.h>
#include <pistache/stream.h>

#ifdef PISTACHE_USE_SSL

#include <openssl/ssl.h>

#endif /* PISTACHE_USE_SSL */

namespace Pistache::Tcp
{

    class Transport;

    class Peer
    {
    public:
        friend class Transport;
        friend class Http::Handler;
        friend class Http::Timeout;

        ~Peer();

        static std::shared_ptr<Peer> Create(Fd fd, const Address& addr);
        static std::shared_ptr<Peer> CreateSSL(Fd fd, const Address& addr, void* ssl);

        // true: there is no http request on the keepalive peer -> only call removePeer
        // false: there is at least one http request on the peer(keepalive or not) -> send 408 message firsst, then call removePeer
        void setIdle(bool bIdle);
        bool isIdle() const;

        const Address& address() const;
        const std::string& hostname();
        Fd fd() const; // can return PS_FD_EMPTY
        em_socket_t actualFd() const; // can return -1

        void closeFd();

        void* ssl() const;

        void putData(std::string name, std::shared_ptr<void> data);
        std::shared_ptr<void> getData(std::string name) const;
        std::shared_ptr<void> tryGetData(std::string name) const;

        Async::Promise<PST_SSIZE_T> send(const RawBuffer& buffer,
                                         int flags = 0);
        size_t getID() const;

    protected:
        // (provide default constructor so child class ConcretePeer can have
        //  default constructor)
        Peer()
            : id_(0)
        { }

        Peer(Fd fd, const Address& addr, void* ssl);

    private:
        void associateTransport(Transport* transport);
        Transport* transport() const;
        static size_t getUniqueId();

        Transport* transport_ = nullptr;

        Fd fd_ = PS_FD_EMPTY;

        Address addr;

        std::string hostname_;
        std::unordered_map<std::string, std::shared_ptr<void>> data_;

        void* ssl_ = nullptr;
        const size_t id_;
        bool isIdle_ = false;
    };

    std::ostream& operator<<(std::ostream& os, Peer& peer);

} // namespace Pistache::Tcp
