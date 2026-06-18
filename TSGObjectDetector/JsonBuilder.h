#pragma once
#include "types.h"
#include <vector>
#include <string>

namespace JsonBuilder {
    std::string buildJson(const std::vector<DetectionResult>& results);
}
