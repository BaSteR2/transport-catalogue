#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include <utility>
#include <memory>

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace transport_router {

	enum class TypeRoute {
		WAIT,
		RIDE
	};

	struct RouteInfoCustom {
		TypeRoute type = TypeRoute::WAIT;
		std::string name = "";
		int count = 0;
		double time = 0.0;
	};

	class TransportRouter {
	public:
		TransportRouter() = default;
		TransportRouter(graph::DirectedWeightedGraph<double> graph, std::vector<RouteInfoCustom> info);

		const graph::DirectedWeightedGraph<double>& GetGraph() const;
		const graph::Router<double>& GetRouter() const;
		const std::vector<RouteInfoCustom>& GetInfo() const;

		void BuildGraph(double bus_velocity, int bus_wait_time, const catalogue::TransportCatalogue& catalogue);

		void SetGraph(graph::DirectedWeightedGraph<double> graph);
		void SetInfo(std::vector<RouteInfoCustom> info);
		void SetRouter();
		void SetRouterWithData(graph::Router<double>::RoutesInternalData data);

		std::optional<std::vector<RouteInfoCustom>> BuildOptimalRoute(std::size_t vertex1, std::size_t vertex2) const;
	private:
		graph::DirectedWeightedGraph<double> graph_;
		std::vector<RouteInfoCustom> info_from_catalogue_;
		std::unique_ptr<graph::Router<double>> router_ptr_;

		std::vector<std::pair<graph::Edge<double>, RouteInfoCustom>> BuildRoutesRide(const catalogue::TransportCatalogue& catalogue, domain::Bus* bus,
			double bus_velocity, std::size_t begin_bus, std::size_t end_bus);
		std::vector<std::pair<graph::Edge<double>, RouteInfoCustom>> BuildRoutesWait(const catalogue::TransportCatalogue& catalogue, int bus_wait_time);

	};
}