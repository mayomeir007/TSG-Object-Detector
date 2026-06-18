#include "pch.h"
#include "Config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>

AppConfig Config::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[TSG] ERROR: Cannot open config file: " << path << "\n";
        std::exit(1);
    }

    AppConfig cfg;
    cfg.host = "0.0.0.0";
    cfg.port = 8080;

    std::string line;
    while (std::getline(file, line)) {
        // Strip whitespace
        auto trim = [](const std::string& s) {
            size_t start = s.find_first_not_of(" \t\r\n");
            size_t end = s.find_last_not_of(" \t\r\n");
            return (start == std::string::npos) ? std::string{} : s.substr(start, end - start + 1);
        };

        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == '[')
            continue;

        auto eq = trimmed.find('=');
        if (eq == std::string::npos) {
            std::cerr << "[TSG] ERROR: Malformed config line: " << line << "\n";
            std::exit(1);
        }

        std::string key = trim(trimmed.substr(0, eq));
        std::string val = trim(trimmed.substr(eq + 1));

        if (key == "port") {
            try { cfg.port = std::stoi(val); }
            catch (...) {
                std::cerr << "[TSG] ERROR: Invalid port value: " << val << "\n";
                std::exit(1);
            }
        } else if (key == "host") {
            cfg.host = val;
        }
    }

    return cfg;
}
