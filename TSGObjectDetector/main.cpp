#include "pch.h"
#include "Config.h"
#include "Server.h"
#include <iostream>

int main(int argc, char* argv[]) {
    AppConfig cfg = Config::load("config.ini");

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--save-dir" && i + 1 < argc) {
            cfg.save_dir = argv[++i];
        } else {
            std::cerr << "[TSG] WARNING: Unknown argument: " << arg << "\n";
        }
    }

    std::cout << "[TSG] Server listening on " << cfg.host << ":" << cfg.port << "\n";
    if (!cfg.save_dir.empty())
        std::cout << "[TSG] Annotated images will be saved to: " << cfg.save_dir << "\n";

    Server server(cfg);
    server.run();
    return 0;
}
