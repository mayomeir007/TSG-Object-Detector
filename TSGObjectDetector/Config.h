#pragma once
#include <string>

struct AppConfig {
    std::string host;
    int port;
    std::string save_dir;   // empty = don't save annotated images
};

namespace Config {
    AppConfig load(const std::string& path);
}
