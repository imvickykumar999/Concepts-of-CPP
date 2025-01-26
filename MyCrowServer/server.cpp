#include "crow_all.h" // Include the Crow library

int main() {
    crow::SimpleApp app; // Create a Crow application

    // Define a route for the home page
    CROW_ROUTE(app, "/")([]() {
        return "Hello, World! Welcome to my C++ web server.";
    });

    // Define a route with parameters
    CROW_ROUTE(app, "/greet/<string>")([](std::string name) {
        return "Hello, " + name + "! This is your personalized greeting.";
    });

    // Start the server on port 8080
    app.port(8080).multithreaded().run();
}
