#pragma once
#include "types.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

namespace Annotator {
    // Draws bounding boxes on a copy of the image and writes it to save_dir.
    // Returns the full path of the saved file, or empty string on failure.
    std::string save(const cv::Mat& image,
                     const std::vector<DetectionResult>& results,
                     const std::string& save_dir);
}
