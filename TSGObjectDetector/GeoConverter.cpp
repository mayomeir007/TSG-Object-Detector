#include "pch.h"
#include "GeoConverter.h"
#include <cmath>

GeoCoord GeoConverter::toLatLon(int px_x, int px_y, GeoCoord top_left, double meters_per_pixel) {
    constexpr double METERS_PER_DEGREE_LAT = 111320.0;

    double delta_lat = (px_y * meters_per_pixel) / METERS_PER_DEGREE_LAT;
    double delta_lon = (px_x * meters_per_pixel) /
        (METERS_PER_DEGREE_LAT * std::cos(top_left.latitude * M_PI / 180.0));

    return { top_left.latitude - delta_lat, top_left.longitude + delta_lon };
}
