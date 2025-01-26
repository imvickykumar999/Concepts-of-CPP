#include <httpserver.hpp>

class MyResource : public httpserver::http_resource {
public:
    const std::shared_ptr<httpserver::http_response> render_GET(const httpserver::http_request&) override {
        return std::make_shared<httpserver::string_response>("Hello, World!", 200, "text/plain");
    }
};

int main() {
    httpserver::webserver ws = httpserver::create_webserver(8080);
    MyResource my_resource;
    ws.register_resource("/hello", &my_resource);
    ws.start();
    return 0;
}
