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

    auto trim = [](const std::string& s) {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end   = s.find_last_not_of(" \t\r\n");
        return (start == std::string::npos) ? std::string{} : s.substr(start, end - start + 1);
    };

    std::string line;
    std::string section;
    while (std::getline(file, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#')
            continue;

        if (trimmed[0] == '[') {
            auto close = trimmed.find(']');
            if (close != std::string::npos)
                section = trim(trimmed.substr(1, close - 1));
            continue;
        }

        auto eq = trimmed.find('=');
        if (eq == std::string::npos) {
            std::cerr << "[TSG] ERROR: Malformed config line: " << line << "\n";
            std::exit(1);
        }

        std::string key = trim(trimmed.substr(0, eq));
        std::string val = trim(trimmed.substr(eq + 1));

        if (section == "server") {
            if (key == "port") {
                try { cfg.port = std::stoi(val); }
                catch (...) {
                    std::cerr << "[TSG] ERROR: Invalid port value: " << val << "\n";
                    std::exit(1);
                }
            } else if (key == "host") {
                cfg.host = val;
            }
        } else if (section == "detector") {
            if (key == "model") {
                cfg.model_path = val;
            } else if (key == "confidence") {
                try { cfg.conf_threshold = std::stof(val); }
                catch (...) {
                    std::cerr << "[TSG] ERROR: Invalid confidence value: " << val << "\n";
                    std::exit(1);
                }
            } else if (key == "nms") {
                try { cfg.nms_threshold = std::stof(val); }
                catch (...) {
                    std::cerr << "[TSG] ERROR: Invalid nms value: " << val << "\n";
                    std::exit(1);
                }
            } else if (key == "building_class") {
                try { cfg.building_class = std::stoi(val); }
                catch (...) {
                    std::cerr << "[TSG] ERROR: Invalid building_class value: " << val << "\n";
                    std::exit(1);
                }
            }
        }
    }

    return cfg;
}
