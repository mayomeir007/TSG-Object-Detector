#include "pch.h"
#include "Server.h"
#include "Detector.h"
#include "GeoConverter.h"
#include "JsonBuilder.h"
#include "Annotator.h"
#include <httplib.h>
#include <opencv2/opencv.hpp>
#include <iostream>

Server::Server(const AppConfig& cfg) : m_cfg(cfg) {}

void Server::run() {
    httplib::Server svr;

    const std::string& save_dir = m_cfg.save_dir;

    svr.Post("/detect", [&save_dir](const httplib::Request& req, httplib::Response& res) {
        // Validate required fields
        if (!req.has_file("image") || !req.has_file("lat") ||
            !req.has_file("lon") || !req.has_file("mpp")) {
            res.status = 400;
            res.set_content("Missing required field(s): image, lat, lon, mpp", "text/plain");
            return;
        }

        double lat, lon, mpp;
        try {
            lat = std::stod(req.get_file_value("lat").content);
            lon = std::stod(req.get_file_value("lon").content);
            mpp = std::stod(req.get_file_value("mpp").content);
        } catch (...) {
            res.status = 400;
            res.set_content("Invalid numeric field(s): lat, lon, mpp must be numbers", "text/plain");
            return;
        }

        const auto& image_file = req.get_file_value("image");
        std::vector<uchar> buf(image_file.content.begin(), image_file.content.end());
        cv::Mat mat = cv::imdecode(buf, cv::IMREAD_COLOR);
        if (mat.empty()) {
            res.status = 400;
            res.set_content("Invalid or unreadable image", "text/plain");
            return;
        }

        GeoCoord top_left{ lat, lon };
        auto boxes = Detector::detect(mat);

        std::vector<DetectionResult> detections;
        detections.reserve(boxes.size());
        int id = 1;
        for (const auto& box : boxes) {
            int cx = box.x + box.w / 2;
            int cy = box.y + box.h / 2;
            GeoCoord loc = GeoConverter::toLatLon(cx, cy, top_left, mpp);
            detections.push_back({ id++, box, loc });
        }

        if (!save_dir.empty()) {
            std::string out = Annotator::save(mat, detections, save_dir);
            if (!out.empty())
                std::cout << "[TSG] Annotated image saved to: " << out << "\n";
        }

        std::string json = JsonBuilder::buildJson(detections);
        res.set_content(json, "application/json");
    });

    std::cout << "[TSG] Listening on " << m_cfg.host << ":" << m_cfg.port << "\n";
    svr.listen(m_cfg.host.c_str(), m_cfg.port);
}
