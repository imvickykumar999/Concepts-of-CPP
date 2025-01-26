/*
 * SPDX-FileCopyrightText: 2018 Pablo Ghiglino
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/peer.h>
#include <pistache/router.h>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <httplib.h>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif


#include <chrono>
#include <thread>

#ifdef _WIN32
#include <versionhelpers.h> // used for choosing certain timeouts
#endif

using namespace std;
using namespace Pistache;

static constexpr auto KEEPALIVE_TIMEOUT = std::chrono::seconds(2);

class StatsEndpoint
{
public:
    StatsEndpoint(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    { }

    void init(size_t thr = 2)
    {
        auto opts = Http::Endpoint::options().threads(static_cast<int>(thr));
        opts.keepaliveTimeout(KEEPALIVE_TIMEOUT);
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start()
    {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serveThreaded();
    }

    void shutdown() { httpEndpoint->shutdown(); }

    Port getPort() const { return httpEndpoint->getPort(); }

    std::vector<std::shared_ptr<Tcp::Peer>> getAllPeer()
    {
        return httpEndpoint->getAllPeer();
    }

private:
    void setupRoutes()
    {
        using namespace Rest;
        Routes::Get(router, "/read/function1",
                    Routes::bind(&StatsEndpoint::doAuth, this));
        Routes::Get(router, "/read/hostname",
                    Routes::bind(&StatsEndpoint::doResolveClient, this));
    }

    void doAuth(const Rest::Request& /*request*/,
                Http::ResponseWriter response)
    {
        std::thread worker(
            [](Http::ResponseWriter writer) { writer.send(Http::Code::Ok, "1"); },
            std::move(response));
        worker.detach();
    }

    void doResolveClient(const Rest::Request& /*request*/,
                         Http::ResponseWriter response)
    {
        response.send(Http::Code::Ok, response.peer()->hostname());
    }

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};

TEST(rest_server_test, basic_test)
{
    int thr = 1;

    Address addr(Ipv4::any(), Port(0));

    StatsEndpoint stats(addr);

    stats.init(thr);
    stats.start();
    Port port = stats.getPort();

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;
    cout << "Port = " << port << endl;

    httplib::Client client("localhost", port);
    auto res = client.Get("/read/function1");
    ASSERT_EQ(res->status, 200);
    ASSERT_EQ(res->body, "1");

    res = client.Get("/read/hostname");
    ASSERT_EQ(res->status, 200);

    // TODO: Clean this up to use proper gtest macros.
    if ((res->body.length() >= 9) && // 9 being len of "localhost"
        (0 == res->body.compare(res->body.length() - 9, 9, "localhost")))
    {
        // NOTE: res->body is "localhost", or "ip6-localhost", or (seen on
        // Windows 10) "view-localhost" on some architectures.
        // We are checking for res->body ends in "localhost"
        ASSERT_TRUE(
            0 == res->body.compare(res->body.length() - 9, 9, "localhost"));
    }
    else
    {
        const unsigned int my_max_hostname_len = 1024;

        // NetBSD showed this case, when hostname was not "localhost"
        char name[my_max_hostname_len + 6];
        name[0] = 0;

        int ghn_res = gethostname(&(name[0]), my_max_hostname_len+2);
        ASSERT_EQ(ghn_res, 0);

        ASSERT_EQ(res->body, &(name[0]));
    }

    stats.shutdown();
}

TEST(rest_server_test, response_status_code_test)
{
    int thr = 1;

    Address addr(Ipv4::any(), Port(0));

    StatsEndpoint stats(addr);

    stats.init(thr);
    stats.start();
    Port port = stats.getPort();

    cout << "Cores = " << hardware_concurrency() << endl;
    cout << "Using " << thr << " threads" << endl;
    cout << "Port = " << port << endl;

    httplib::Client client("localhost", port);

    // Code 404 - Not Found.
    auto res = client.Get("/read/does_not_exist");
    EXPECT_EQ(res->status, 404);
    EXPECT_EQ(res->body, "Could not find a matching route");

    // Code 405 - Method Not Allowed.
    std::string body("body goes here");
    res = client.Post("/read/function1", body, "text/plain");
    EXPECT_EQ(res->status, 405);
    EXPECT_EQ(res->body, "Method Not Allowed");
    ASSERT_TRUE(res->has_header("Allow"));
    EXPECT_EQ(res->get_header_value("Allow"), "GET");

    // Code 415 - Unknown Media Type
    res = client.Post("/read/function1", body, "invalid");
    EXPECT_EQ(res->status, 415);
    EXPECT_EQ(res->body, "Unknown Media Type");

    stats.shutdown();
}

TEST(rest_server_test, keepalive_server_timeout)
{
    int thr = 1;
    Address addr(Ipv4::loopback(), Port(0));
    StatsEndpoint stats(addr);
    stats.init(thr);
    stats.start();
    Port port = stats.getPort();

    httplib::Client client("localhost", port);
    client.set_keep_alive(true);
    auto peer = stats.getAllPeer();

    // first request
    auto res = client.Get("/read/hostname");
    EXPECT_EQ(res->status, 200);
    peer           = stats.getAllPeer();
    auto peerPort1 = (*peer.begin())->address().port();
    EXPECT_EQ(peer.size(), 1ul);

    // second request
    res = client.Get("/read/hostname");
    EXPECT_EQ(res->status, 200);
    peer           = stats.getAllPeer();
    auto peerPort2 = (*peer.begin())->address().port();
    // first and second use the same connection
    EXPECT_EQ(peerPort1, peerPort2);

    // The server checks the connection status once every 500 milliseconds.
    // wait for timeout, check whether the server has closed the connection
    std::this_thread::sleep_for(KEEPALIVE_TIMEOUT + std::chrono::milliseconds(700));
    peer = stats.getAllPeer();
    EXPECT_EQ(peer.size(), 0ul);

    stats.shutdown();
}

TEST(rest_server_test, keepalive_client_timeout)
{
    int thr = 1;
    Address addr(Ipv4::loopback(), Port(0));
    StatsEndpoint stats(addr);
    stats.init(thr);
    stats.start();
    Port port = stats.getPort();

    {
        httplib::Client client("localhost", port);
        client.set_keep_alive(true);

        auto res = client.Get("/read/hostname");
        EXPECT_EQ(res->status, 200);
        auto peer = stats.getAllPeer();
        EXPECT_EQ(peer.size(), 1ul);
        // The client actively closes the connection
    }
    // The server checks the connection status once every 500 milliseconds.
    // check whether the server has closed the connection
    std::this_thread::sleep_for(std::chrono::milliseconds(700));
    auto peer = stats.getAllPeer();
    EXPECT_EQ(peer.size(), 0ul);

    stats.shutdown();
}

TEST(rest_server_test, keepalive_multithread_client_request)
{
    int thr = 1;
    Address addr(Ipv4::loopback(), Port(0));
    StatsEndpoint stats(addr);
    stats.init(thr);
    stats.start();
    Port port = stats.getPort();

    unsigned long THR_NUMBER = 10;
    std::vector<std::thread> threads;
    for (unsigned long i = 0; i < THR_NUMBER; ++i)
    {
        threads.push_back(std::thread([&port] {
            httplib::Client client("localhost", port);
            client.set_keep_alive(true);

            auto res = client.Get("/read/hostname");
            EXPECT_EQ(res->status, 200);
            std::this_thread::sleep_for(KEEPALIVE_TIMEOUT + std::chrono::milliseconds(700));
        }));
    }
    // @Sept/2024. Based on behavior seen in Windows 11, changed this timeout
    // from 700ms to 3500ms.
    //
    // The longer timeout is required due to the behavior of httplib.h
    // client.Get. In overview, httplib.h loops through the client addr it
    // finds, issuing a "connect" for each addr in series. Following each
    // "connect", it calls wait_until_socket_is_ready, which uses "select" with
    // a 300ms timeout to get a result for the connect. Now, suppose there
    // were 10 addr that failed select based on the timeout, it would take
    // 3000ms until finally connect was issued on the addr that works.
    //
    // On the server side, the peer cannot be created until the "connect" is
    // received. So if the sleep_for below were 700ms, there would still be
    // zero server-side peers once the sleep_for completes, causing:
    //   EXPECT_EQ(peer.size(), THR_NUMBER)
    // to fail below.
    //
    // In practice, we saw ~2200ms delay before the socket connected
    // successfully - Windows 11 running in a VM. So in Windows, this test
    // would fail with a 700ms sleep_for, but succeed with 3500ms.
    //
    // Conversely, for macOS if we use 3500ms then the test fails - presumably,
    // the connect happens so fast that not only has the connect happened but
    // also the clients connections have timed out and the peers at the server
    // have all closed again before the EXPECT_EQ(peer.size(), THR_NUMBER) test
    // below executes.
    //
    // Experimentation with Windows 2019 server (using Visual Studio
    // 2019), indicates further variability in the correct sleep time ahead of
    // testing that peer.size() equals THR_NUMBER.
    // In Debug build, it works with sleep times between 3000 and 3750ms.
    // In Release build, it works with sleep times between 1063 and 3063.
    //
    // Given this level of variability in the time taken to connect the peers,
    // we make the sleep time dynamic, checking every 700ms to see if the peers
    // have successfully connected.

    size_t max_peers_seen = 0;
    for(unsigned int i=0; i<8; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(700));
        auto peer = stats.getAllPeer();
        if (peer.size() > max_peers_seen)
        {
            max_peers_seen = peer.size();
            if (max_peers_seen >= THR_NUMBER)
                break;
        }
    }
    EXPECT_EQ(max_peers_seen, THR_NUMBER);

    for (auto it = threads.begin(); it != threads.end(); ++it)
    {
        it->join();
    }
    auto peer = stats.getAllPeer();
    EXPECT_EQ(peer.size(), 0ul);

    stats.shutdown();
}
