#include "pch.h"
#include "Detector.h"
#include <onnxruntime_cxx_api.h>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

static constexpr int   MODEL_SIZE = 640;
static constexpr float PAD_VALUE  = 114.0f;

namespace {
    Ort::Env*     g_env     = nullptr;
    Ort::Session* g_session = nullptr;

    float g_conf_threshold  = 0.5f;
    float g_nms_threshold   = 0.45f;
    int   g_building_class  = 0;
    int   g_num_classes     = 1;

    std::vector<std::string> g_input_names_storage;
    std::vector<std::string> g_output_names_storage;
    std::vector<const char*> g_input_names;
    std::vector<const char*> g_output_names;

    struct LetterboxInfo {
        float scale;
        int   pad_x;
        int   pad_y;
    };

    cv::Mat letterbox(const cv::Mat& src, LetterboxInfo& info) {
        float scale_w = static_cast<float>(MODEL_SIZE) / src.cols;
        float scale_h = static_cast<float>(MODEL_SIZE) / src.rows;
        info.scale    = std::min(scale_w, scale_h);

        int new_w = static_cast<int>(std::round(src.cols * info.scale));
        int new_h = static_cast<int>(std::round(src.rows * info.scale));
        info.pad_x = (MODEL_SIZE - new_w) / 2;
        info.pad_y = (MODEL_SIZE - new_h) / 2;

        cv::Mat resized;
        cv::resize(src, resized, cv::Size(new_w, new_h), 0, 0, cv::INTER_LINEAR);

        cv::Mat out(MODEL_SIZE, MODEL_SIZE, CV_8UC3,
                    cv::Scalar(static_cast<uchar>(PAD_VALUE),
                               static_cast<uchar>(PAD_VALUE),
                               static_cast<uchar>(PAD_VALUE)));
        resized.copyTo(out(cv::Rect(info.pad_x, info.pad_y, new_w, new_h)));
        return out;
    }
}

void Detector::init(const AppConfig& cfg) {
    if (cfg.model_path.empty()) {
        std::cerr << "[TSG] ERROR: No model path configured in [detector] section.\n";
        std::exit(1);
    }

    g_conf_threshold = cfg.conf_threshold;
    g_nms_threshold  = cfg.nms_threshold;
    g_building_class = cfg.building_class;

    g_env = new Ort::Env(ORT_LOGGING_LEVEL_WARNING, "TSGDetector");

    Ort::SessionOptions opts;
    opts.SetIntraOpNumThreads(1);
    opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    std::wstring wpath(cfg.model_path.begin(), cfg.model_path.end());
    g_session = new Ort::Session(*g_env, wpath.c_str(), opts);

    Ort::AllocatorWithDefaultOptions alloc;

    size_t num_inputs = g_session->GetInputCount();
    for (size_t i = 0; i < num_inputs; ++i) {
        auto name = g_session->GetInputNameAllocated(i, alloc);
        g_input_names_storage.push_back(name.get());
    }
    for (const auto& n : g_input_names_storage)
        g_input_names.push_back(n.c_str());

    size_t num_outputs = g_session->GetOutputCount();
    for (size_t i = 0; i < num_outputs; ++i) {
        auto name = g_session->GetOutputNameAllocated(i, alloc);
        g_output_names_storage.push_back(name.get());
    }
    for (const auto& n : g_output_names_storage)
        g_output_names.push_back(n.c_str());

    // Infer num_classes from output shape: [1, 4+N, 8400]
    auto out_info  = g_session->GetOutputTypeInfo(0);
    auto shape     = out_info.GetTensorTypeAndShapeInfo().GetShape();
    if (shape.size() == 3 && shape[1] > 4)
        g_num_classes = static_cast<int>(shape[1]) - 4;

    std::cout << "[TSG] ONNX model loaded: " << cfg.model_path
              << "  classes=" << g_num_classes << "\n";
}

std::vector<BoundingBox> Detector::detect(const cv::Mat& image) {
    if (!g_session)
        throw std::runtime_error("Detector::init() was not called");

    LetterboxInfo lb;
    cv::Mat lb_img = letterbox(image, lb);

    // BGR -> RGB, normalize, HWC -> CHW
    cv::Mat rgb;
    cv::cvtColor(lb_img, rgb, cv::COLOR_BGR2RGB);
    rgb.convertTo(rgb, CV_32F, 1.0f / 255.0f);

    std::vector<float> tensor(3 * MODEL_SIZE * MODEL_SIZE);
    std::vector<cv::Mat> channels(3);
    cv::split(rgb, channels);
    for (int c = 0; c < 3; ++c)
        std::memcpy(tensor.data() + c * MODEL_SIZE * MODEL_SIZE,
                    channels[c].ptr<float>(),
                    MODEL_SIZE * MODEL_SIZE * sizeof(float));

    std::array<int64_t, 4> input_shape = {1, 3, MODEL_SIZE, MODEL_SIZE};
    auto mem_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        mem_info, tensor.data(), tensor.size(),
        input_shape.data(), input_shape.size());

    auto outputs = g_session->Run(
        Ort::RunOptions{nullptr},
        g_input_names.data(),  &input_tensor, 1,
        g_output_names.data(), g_output_names.size());

    // Output shape: [1, 4+num_classes, 8400]
    float* data = outputs[0].GetTensorMutableData<float>();
    auto   out_shape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
    int64_t rows    = out_shape[1]; // 4 + num_classes
    int64_t anchors = out_shape[2]; // 8400

    std::vector<cv::Rect>  boxes;
    std::vector<float>     scores;

    for (int64_t a = 0; a < anchors; ++a) {
        // data is stored [row][anchor], i.e. col-major for each row
        float cx = data[0 * anchors + a];
        float cy = data[1 * anchors + a];
        float w  = data[2 * anchors + a];
        float h  = data[3 * anchors + a];

        float score = 0.0f;
        if (g_building_class < g_num_classes)
            score = data[(4 + g_building_class) * anchors + a];

        if (score < g_conf_threshold)
            continue;

        // Convert from 640-space back to original image coords
        float x1 = (cx - w / 2.0f - lb.pad_x) / lb.scale;
        float y1 = (cy - h / 2.0f - lb.pad_y) / lb.scale;
        float bw = w / lb.scale;
        float bh = h / lb.scale;

        // Clamp to image bounds
        x1 = std::max(0.0f, x1);
        y1 = std::max(0.0f, y1);
        bw = std::min(bw, static_cast<float>(image.cols) - x1);
        bh = std::min(bh, static_cast<float>(image.rows) - y1);

        boxes.push_back(cv::Rect(static_cast<int>(x1), static_cast<int>(y1),
                                  static_cast<int>(bw), static_cast<int>(bh)));
        scores.push_back(score);
    }

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, scores, g_conf_threshold, g_nms_threshold, indices);

    std::vector<BoundingBox> results;
    results.reserve(indices.size());
    for (int idx : indices) {
        const cv::Rect& r = boxes[idx];
        results.push_back({r.x, r.y, r.width, r.height});
    }
    return results;
}
