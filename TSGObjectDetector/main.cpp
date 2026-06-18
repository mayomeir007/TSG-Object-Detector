#include "pch.h"
#include "Config.h"
#include "Server.h"
#include <iostream>

int main() {
    AppConfig cfg = Config::load("config.ini");
    std::cout << "[TSG] Server listening on " << cfg.host << ":" << cfg.port << "\n";
    Server server(cfg);
    server.run();
    return 0;
}
