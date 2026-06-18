#include "pch.h"
#include "JsonBuilder.h"
#include <nlohmann/json.hpp>

std::string JsonBuilder::buildJson(const std::vector<DetectionResult>& results) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& r : results) {
        arr.push_back({
            {"object_id", r.id},
            {"bounding_box", {r.bbox.x, r.bbox.y, r.bbox.w, r.bbox.h}},
            {"location", {{"latitude", r.location.latitude}, {"longitude", r.location.longitude}}}
        });
    }
    return arr.dump(2);
}
