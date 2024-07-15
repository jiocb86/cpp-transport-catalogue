#pragma once

#include <string>
#include <vector>

#include "geo.h"

namespace catalogue {
struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Bus {
    std::string number;
    std::vector<const Stop*> route;
    bool is_circle;
};

struct BusInfo {
    int stops_count;
    int unique_stops_count;
    double geo_length;
    int dist_length;
};
} // namespace catalogue
