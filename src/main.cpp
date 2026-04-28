#include "../includes/Server.hpp"

int main() {
    Server server(6667, "pass");

    server.setupSocket();
    server.start();
}