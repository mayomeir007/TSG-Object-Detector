#pragma once
#include "Config.h"
#include "types.h"
#include <opencv2/opencv.hpp>
#include <vector>

namespace Detector {
    void init(const AppConfig& cfg);
    std::vector<BoundingBox> detect(const cv::Mat& image);
}
