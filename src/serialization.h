#pragma once
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <transport_catalogue.pb.h>

namespace serialization {
    catalogue_serialize::Stop SerializeStop(const domain::Stop* stop);
    catalogue_serialize::Bus SerializeBus(const domain::Bus* bus);
	catalogue_serialize::Catalogue SerializeCatalogue(const catalogue::TransportCatalogue& catalogue);
	catalogue_serialize::Renderer SerializeRenderer(const renderer::MapRender& render);
    catalogue_serialize::Edge SerializeEdge(const graph::Edge<double>& edge);
    catalogue_serialize::RouteInfo SerializeInfoRoute(const transport_router::RouteInfoCustom& data);
    catalogue_serialize::Data SerializeRouterData(size_t line, std::optional<graph::Router<double>::RouteInternalData> data);
	catalogue_serialize::Router SerializeRouter(const transport_router::TransportRouter& transport_router);
	void FullSerialize(const std::filesystem::path& path, const catalogue::TransportCatalogue& catalogue, const renderer::MapRender& render, 
		const transport_router::TransportRouter& router);
    
    domain::Stop DeserializeStop(catalogue_serialize::Stop ser_stop);
    domain::DistanceInfo DeserializeDistance(catalogue_serialize::Distance ser_distance);
	void DeserializeCatalogue(const catalogue_serialize::Catalogue& cat_ser, catalogue::TransportCatalogue& catalogue);
	void DeserializeRenderer(const catalogue_serialize::Renderer& rend_ser, renderer::MapRender& render);
    transport_router::RouteInfoCustom DeserializeData(catalogue_serialize::RouteInfo data);
    std::optional<graph::Router<double>::RouteInternalData> DeserializeRouter(catalogue_serialize::Data ser_data);    
	void DeserializeRouter(const catalogue_serialize::Router& rout_ser, transport_router::TransportRouter& router);
	void FullDeserialize(const std::filesystem::path& path, catalogue::TransportCatalogue& catalogue, 
		renderer::MapRender& render, transport_router::TransportRouter& router);

}