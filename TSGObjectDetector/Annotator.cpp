#include "pch.h"
#include "Annotator.h"
#include <chrono>
#include <sstream>
#include <iomanip>

static const cv::Scalar BOX_COLOR   { 0, 255, 0 };   // green
static const cv::Scalar LABEL_BG    { 0, 255, 0 };
static const cv::Scalar LABEL_FG    { 0, 0, 0 };
static constexpr int    BOX_THICKNESS = 2;
static constexpr double FONT_SCALE    = 0.5;

static std::string timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    localtime_s(&tm, &t);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    return ss.str();
}

std::string Annotator::save(const cv::Mat& image,
                            const std::vector<DetectionResult>& results,
                            const std::string& save_dir) {
    cv::Mat annotated = image.clone();

    for (const auto& r : results) {
        cv::Rect rect(r.bbox.x, r.bbox.y, r.bbox.w, r.bbox.h);
        cv::rectangle(annotated, rect, BOX_COLOR, BOX_THICKNESS);

        std::string label = "#" + std::to_string(r.id);
        int baseline = 0;
        cv::Size text_size = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX,
                                              FONT_SCALE, 1, &baseline);

        cv::Point label_origin(rect.x, std::max(rect.y - 2, text_size.height));
        cv::rectangle(annotated,
                      label_origin + cv::Point(0, baseline),
                      label_origin + cv::Point(text_size.width, -text_size.height),
                      LABEL_BG, cv::FILLED);
        cv::putText(annotated, label, label_origin,
                    cv::FONT_HERSHEY_SIMPLEX, FONT_SCALE, LABEL_FG, 1);
    }

    std::string sep = (save_dir.back() == '\\' || save_dir.back() == '/') ? "" : "\\";
    std::string out_path = save_dir + sep + "annotated_" + timestamp() + ".jpg";

    if (!cv::imwrite(out_path, annotated)) {
        std::cerr << "[TSG] WARNING: Failed to save annotated image to: " << out_path << "\n";
        return {};
    }

    return out_path;
}
