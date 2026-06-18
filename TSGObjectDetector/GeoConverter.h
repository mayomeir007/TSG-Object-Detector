#pragma once
#include "types.h"

namespace GeoConverter {
    GeoCoord toLatLon(int px_x, int px_y, GeoCoord top_left, double meters_per_pixel);
}
