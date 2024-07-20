#pragma once

#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include <transport_catalogue.pb.h>
#include "serialization.h"

namespace reader {

	class JsonReader {
	public:
		JsonReader(std::istream& input);

		void FillDataFromJson();
		void SerializeData() const ;
		void DeserializeData();
        
		void PrintRequest(std::ostream& output) const;

	private:
		json::Document json_doc_;

		catalogue::TransportCatalogue catalogue_;
		renderer::MapRender render_;
		transport_router::TransportRouter router_;

		void AddStopFromJson (const json::Node& stop);
		void AddDistanceFromJson(const json::Node& distance);
		void AddBusFromJson(const json::Node& bus);
		void FillCatalogue(const json::Array& base_req);

		svg::Color FillColor(const json::Node& color);
		void FillMapRenderer(const json::Dict& stat_req);

		json::Dict CreateStopRequest(const json::Node& stop_req) const;
		json::Dict CreateBusRequest(const json::Node& bus_req) const;
		json::Dict CreateMapRequest(const json::Node& map_req) const;
		json::Dict CreateRouteRequest(const json::Node& route_req) const;

		void PrintRequestJSON( const json::Array& stat_req, std::ostream& output) const;
	};
}