#pragma once

#include <deque> 
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <set>
#include <vector>
#include <utility>

#include "geo.h"
#include "domain.h"

namespace catalogue {

    class TransportCatalogue {
    public:
        void AddBus(const std::vector<std::string>& bus_for_add, bool is_round);
        void AddStop(const domain::Stop& stop_for_add);
        domain::Bus* FindBus(const std::string_view& bus) const;
        domain::Stop* FindStop(const std::string_view& stop) const;
        domain::BusInfo GetInfoBus(const std::string_view& bus) const ;
        std::set<std::string> GetInfoStop(const std::string_view& stop) const;
        void AddDistanceStops(const domain::DistanceInfo& distance_to_add);
        int GetDistanceStops(const std::string_view& stop1, const std::string_view& stop2) const;
        size_t GetStopsCount() const;
        std::vector<std::string> GetAllBusesName() const;
        std::vector<std::string> GetAllStopsName() const;

    private:
        struct StopHasher {
            size_t operator()(const domain::Stop* stop) const {
                size_t lat_h = double_hasher_(stop->latitude);
                size_t long_h = double_hasher_(stop->longitude);
                size_t n_h = text_hasher_(stop->stop_name);
                return lat_h + long_h * 37 + n_h * (37 * 37);
            }
            std::hash<std::string> text_hasher_;
            std::hash<double> double_hasher_;
        };
        struct DistanceHasher {
            size_t operator()(const std::pair<domain::Stop*, domain::Stop*>& pair_stops) const {
                size_t s1_h = stop_hasher_(pair_stops.first);
                size_t s2_h = stop_hasher_(pair_stops.second);
                return s1_h + s2_h * (37 * 37 * 37);
            }
            StopHasher stop_hasher_;
        };
        std::deque<domain::Stop> stops_;
        std::unordered_map<std::string_view, domain::Stop*> stopname_to_stop_;
        std::deque<domain::Bus> buses_;
        std::unordered_map<std::string_view, domain::Bus*> busname_to_bus_;
        std::unordered_map<std::string_view, std::set<std::string>> stop_buses_;
        std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, int, DistanceHasher> distance_btw_stops_;
    };
}