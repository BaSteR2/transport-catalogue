#pragma once
#include <string>
#include <vector>

namespace domain {

    struct Stop {
        std::string stop_name;
        double latitude;
        double longitude;
        size_t vertex_number = 0;
    };

    struct Bus {
        std::string bus_name;
        std::vector<Stop*> stops_on_bus;
        bool is_round;
    };

    struct BusInfo {
        int stops_on_route;
        int unique_stops;
        int route_length;
        double curvature;
    };

    struct DistanceInfo {
        std::string stop_name;
        std::vector<std::pair<std::string, int>> distance_stops;
    };
}