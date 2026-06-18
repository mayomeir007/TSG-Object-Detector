#include "pch.h"
#include "Detector.h"

static constexpr double MIN_CONTOUR_AREA = 200.0;

std::vector<BoundingBox> Detector::detect(const cv::Mat& image) {
    cv::Mat gray;
    if (image.channels() == 1)
        gray = image.clone();
    else
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    cv::Mat binary;
    cv::threshold(gray, binary, 128, 255, cv::THRESH_BINARY);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<BoundingBox> results;
    for (const auto& contour : contours) {
        if (cv::contourArea(contour) < MIN_CONTOUR_AREA)
            continue;
        cv::Rect r = cv::boundingRect(contour);
        results.push_back({ r.x, r.y, r.width, r.height });
    }

    return results;
}
