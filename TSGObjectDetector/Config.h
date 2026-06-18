#pragma once
#include <string>

struct AppConfig {
    std::string host;
    int port;
};

namespace Config {
    AppConfig load(const std::string& path);
}
