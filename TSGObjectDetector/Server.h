#pragma once
#include "Config.h"

class Server {
public:
    explicit Server(const AppConfig& cfg);
    void run();

private:
    AppConfig m_cfg;
};
