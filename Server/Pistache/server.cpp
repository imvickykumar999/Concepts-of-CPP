#include <pistache/endpoint.h>

using namespace Pistache;

class HelloHandler : public Http::Handler {
public:
    HTTP_PROTOTYPE(HelloHandler)

    void onRequest(const Http::Request&, Http::ResponseWriter response) override {
        response.send(Http::Code::Ok, "Hello, World!");
    }
};

int main() {
    Address addr(Ipv4::any(), Port(8080));
    Http::Endpoint server(addr);

    auto opts = Http::Endpoint::options().threads(1);
    server.init(opts);
    server.setHandler(Http::make_handler<HelloHandler>());
    server.serve();

    return 0;
}
