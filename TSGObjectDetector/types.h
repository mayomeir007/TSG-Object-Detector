#pragma once
#include <string>

struct BoundingBox {
    int x, y, w, h;
};

struct GeoCoord {
    double latitude, longitude;
};

struct DetectionResult {
    int id;
    BoundingBox bbox;
    GeoCoord location;
};

struct RequestParams {
    std::string image_path;
    GeoCoord top_left;
    double meters_per_pixel;
};
