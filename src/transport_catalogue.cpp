#include "transport_catalogue.h"


using namespace std;

namespace catalogue {

    void TransportCatalogue::AddBus(const vector<string>& bus_for_add, bool is_round) {
        domain::Bus added_bus;
        added_bus.bus_name = bus_for_add[0];
        for (size_t i = 1; i < bus_for_add.size(); i++) {
            stop_buses_[bus_for_add[i]].insert(added_bus.bus_name);
            added_bus.stops_on_bus.push_back(stopname_to_stop_.at(bus_for_add[i]));
        }
        added_bus.is_round = is_round;
        buses_.push_back(move(added_bus));
        busname_to_bus_[buses_.back().bus_name] = &buses_.back();
    }

    void TransportCatalogue::AddStop(const domain::Stop& stop_for_add) {
        stops_.push_back(stop_for_add);
        stops_.back().vertex_number = stopname_to_stop_.size();
        stopname_to_stop_[stops_.back().stop_name] = &stops_.back();
        stop_buses_[stops_.back().stop_name];
    }

    domain::Bus* TransportCatalogue::FindBus(const string_view& bus) const {
        if (!busname_to_bus_.count(bus)) {
            return nullptr;
        }
        return busname_to_bus_.at(bus);
    }

    domain::Stop* TransportCatalogue::FindStop(const string_view& stop) const {
        if (!stop_buses_.count(stop)) {
            return nullptr;
        }
        return stopname_to_stop_.at(stop);
    }

    domain::BusInfo TransportCatalogue::GetInfoBus(const string_view& bus) const {
        int count_stops = 1;
        int route_length = 0;
        double curvature = 0;
        unordered_set<string> unique_stops;
        unique_stops.insert(busname_to_bus_.at(bus)->stops_on_bus[0]->stop_name);
        for (size_t i = 1; i < busname_to_bus_.at(bus)->stops_on_bus.size(); i++) {
            curvature += geo::ComputeDistance(
                { busname_to_bus_.at(bus)->stops_on_bus[i - 1]->latitude, busname_to_bus_.at(bus)->stops_on_bus[i - 1]->longitude },
                { busname_to_bus_.at(bus)->stops_on_bus[i]->latitude, busname_to_bus_.at(bus)->stops_on_bus[i]->longitude });
            route_length += GetDistanceStops(busname_to_bus_.at(bus)->stops_on_bus[i - 1]->stop_name, busname_to_bus_.at(bus)->stops_on_bus[i]->stop_name);
            unique_stops.insert(busname_to_bus_.at(bus)->stops_on_bus[i]->stop_name);
            count_stops++;
        }
        return { count_stops, static_cast<int>(unique_stops.size()), route_length, route_length / curvature };
    }

    set<string> TransportCatalogue::GetInfoStop(const string_view& stop) const {
        return stop_buses_.at(stop);
    }

    void TransportCatalogue::AddDistanceStops(const domain::DistanceInfo& distance_to_add) {
        for (auto& [stop, distance] : distance_to_add.distance_stops) {
            distance_btw_stops_[{ stopname_to_stop_.at(distance_to_add.stop_name), stopname_to_stop_.at(stop) }] = distance;
        }
    }

    int TransportCatalogue::GetDistanceStops(const string_view& stop1, const string_view& stop2) const {
        if (distance_btw_stops_.count(pair{ stopname_to_stop_.at(stop1), stopname_to_stop_.at(stop2) })) {
            return distance_btw_stops_.at(pair{ stopname_to_stop_.at(stop1), stopname_to_stop_.at(stop2) });
        }
        else if(distance_btw_stops_.count(pair{ stopname_to_stop_.at(stop2), stopname_to_stop_.at(stop1) })){
            return distance_btw_stops_.at(pair{ stopname_to_stop_.at(stop2), stopname_to_stop_.at(stop1) });
        }
        else {
            return 0;
        }
    }

    size_t TransportCatalogue::GetStopsCount() const {
        return stopname_to_stop_.size();
    }

    vector<string> TransportCatalogue::GetAllBusesName() const {
        vector<string> buses_for_out;
        for (auto& bus : buses_) {
            buses_for_out.push_back(bus.bus_name);
        }
        return buses_for_out;
    }

    std::vector<std::string> TransportCatalogue::GetAllStopsName() const {
        vector<string> stops_for_out;
        for (auto& stop : stops_) {
            stops_for_out.push_back(stop.stop_name);
        }
        return stops_for_out;
    }
}