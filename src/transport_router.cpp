#include "transport_router.h"

using namespace std;

namespace transport_router {

    const double CONVERT_COEF = 0.06;
    std::vector<std::pair<graph::Edge<double>, RouteInfoCustom>> TransportRouter::BuildRoutesRide(const catalogue::TransportCatalogue& catalogue, domain::Bus* bus,
        double bus_velocity, size_t begin_bus, size_t end_bus) {
        std::vector<std::pair<graph::Edge<double>, RouteInfoCustom>> route_info;
        for (size_t i = begin_bus; i < end_bus; i++) {
            int distance = 0;
            int span_count = 0;
            for (size_t j = i; j < end_bus; j++) {
                distance += catalogue.GetDistanceStops(bus->stops_on_bus[j - 1]->stop_name, bus->stops_on_bus[j]->stop_name);
                span_count++;
                double result_time = distance / bus_velocity * CONVERT_COEF;
                route_info.push_back({ { bus->stops_on_bus[i - 1]->vertex_number * 2 + 1, bus->stops_on_bus[j]->vertex_number * 2, result_time },
                                       { TypeRoute::RIDE, bus->bus_name, span_count, result_time } });
            }
        }
        return route_info;
    }

    std::vector<std::pair<graph::Edge<double>, RouteInfoCustom>> TransportRouter::BuildRoutesWait(const catalogue::TransportCatalogue& catalogue,
        int bus_wait_time) {
        std::vector<std::pair<graph::Edge<double>, RouteInfoCustom>> route_info;
        vector<string> stop_names = catalogue.GetAllStopsName();
        for (auto& stop_name : stop_names) {
            domain::Stop* stop = catalogue.FindStop(stop_name);
            size_t vertex = stop->vertex_number * 2;
            route_info.push_back({ { vertex, vertex + 1, static_cast<double>(bus_wait_time) },
                                   { TypeRoute::WAIT, stop->stop_name, 0,  static_cast<double>(bus_wait_time) } });
        }
        return route_info;
    }

    void TransportRouter::BuildGraph(double bus_velocity, int bus_wait_time, const catalogue::TransportCatalogue& catalogue) {
        size_t stop_count = catalogue.GetStopsCount();
        graph::DirectedWeightedGraph<double> graph(stop_count * 2);
        std::vector<std::pair<graph::Edge<double>, RouteInfoCustom>> edges_for_add;
        vector<string> bus_names = catalogue.GetAllBusesName();
        for (auto& bus_name : bus_names) {
            domain::Bus* bus = catalogue.FindBus(bus_name);
            if (bus->is_round) {
                std::vector<std::pair<graph::Edge<double>, RouteInfoCustom>> round_route
                    = BuildRoutesRide(catalogue, bus, bus_velocity, 1, bus->stops_on_bus.size());
                edges_for_add.insert(edges_for_add.end(), round_route.begin(), round_route.end());
            }
            else {
                std::vector<std::pair<graph::Edge<double>, RouteInfoCustom>> forward_route
                    = BuildRoutesRide(catalogue, bus, bus_velocity, 1, bus->stops_on_bus.size() / 2 + 1);
                edges_for_add.insert(edges_for_add.end(), forward_route.begin(), forward_route.end());
                std::vector<std::pair<graph::Edge<double>, RouteInfoCustom>> back_route
                    = BuildRoutesRide(catalogue, bus, bus_velocity, bus->stops_on_bus.size() / 2 + 1, bus->stops_on_bus.size());
                edges_for_add.insert(edges_for_add.end(), back_route.begin(), back_route.end());
            }
        }

        std::vector<std::pair<graph::Edge<double>, RouteInfoCustom>> stops_wait
            = BuildRoutesWait(catalogue, bus_wait_time);
        edges_for_add.insert(edges_for_add.end(), stops_wait.begin(), stops_wait.end());

        info_from_catalogue_.resize(edges_for_add.size());
        for (auto& [edge, info] : edges_for_add) {
            info_from_catalogue_[graph.AddEdge(edge)] = info;
        }
        graph_ = std::move(graph);
    }

    TransportRouter::TransportRouter(graph::DirectedWeightedGraph<double> graph, std::vector<RouteInfoCustom> info)
        : graph_(move(graph)), info_from_catalogue_(info)
    {
    }

    const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
        return graph_;
    }

    const graph::Router<double>& TransportRouter::GetRouter() const {
        return *router_ptr_;
    }

    const std::vector<RouteInfoCustom>& TransportRouter::GetInfo() const {
        return info_from_catalogue_;
    }

    void TransportRouter::SetRouter() {
        router_ptr_ = std::make_unique<graph::Router<double>>(graph_);
    }
    
    void TransportRouter::SetRouterWithData(graph::Router<double>::RoutesInternalData data) {
        router_ptr_ = std::make_unique<graph::Router<double>>(graph_, data);
    }

    void TransportRouter::SetGraph(graph::DirectedWeightedGraph<double> graph) {
        graph_ = std::move(graph);
    }

    void TransportRouter::SetInfo(std::vector<RouteInfoCustom> info) {
        info_from_catalogue_ = std::move(info);
    }

    std::optional<vector<RouteInfoCustom>> TransportRouter::BuildOptimalRoute(size_t vertex1, size_t vertex2) const {
        std::optional<graph::Router<double>::RouteInfo> optimal_route = router_ptr_->BuildRoute(vertex1, vertex2);
        if (!optimal_route) {
            return nullopt;
        }
        std::vector<RouteInfoCustom> result;
        result.push_back({ TypeRoute::WAIT, "", 0, optimal_route->weight });
        for (auto& edge : optimal_route->edges) {
            result.push_back(info_from_catalogue_[edge]);
        }
        return result;
    }
}