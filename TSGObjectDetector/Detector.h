#pragma once
#include "types.h"
#include <opencv2/opencv.hpp>
#include <vector>

namespace Detector {
    std::vector<BoundingBox> detect(const cv::Mat& image);
}
