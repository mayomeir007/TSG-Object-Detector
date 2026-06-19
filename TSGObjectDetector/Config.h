#pragma once
#include <string>

struct AppConfig {
    std::string host;
    int port;
    std::string save_dir;   // empty = don't save annotated images

    std::string model_path;
    float conf_threshold = 0.5f;
    float nms_threshold  = 0.45f;
    int   building_class = 0;
};

namespace Config {
    AppConfig load(const std::string& path);
}
