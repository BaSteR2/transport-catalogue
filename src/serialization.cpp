#include "serialization.h"

namespace serialization {

	catalogue_serialize::Stop SerializeStop(const domain::Stop* stop) {
		catalogue_serialize::Stop stop_ser;
		stop_ser.set_name(stop->stop_name);
		stop_ser.set_lati(stop->latitude);
		stop_ser.set_longi(stop->longitude);
		stop_ser.set_vertex(stop->vertex_number);
		return stop_ser;
	}

	catalogue_serialize::Bus SerializeBus(const domain::Bus* bus) {
		catalogue_serialize::Bus bus_ser;
		*bus_ser.add_names() = bus->bus_name;
		for (auto& stop : bus->stops_on_bus) {
			*bus_ser.add_names() = stop->stop_name;
		}
		bus_ser.set_is_round(bus->is_round);
		return bus_ser;
	}

	catalogue_serialize::Catalogue SerializeCatalogue(const catalogue::TransportCatalogue& catalogue) {
		catalogue_serialize::Catalogue cat_ser;
		for (auto& stop_name : catalogue.GetAllStopsName()) {
			domain::Stop* stop = catalogue.FindStop(stop_name);
			*cat_ser.add_stops() = SerializeStop(stop);
		}
		for (auto& bus_name : catalogue.GetAllBusesName()) {
			domain::Bus* bus = catalogue.FindBus(bus_name);
			*cat_ser.add_buses() = SerializeBus(bus);
		}
		for (auto& from : catalogue.GetAllStopsName()) {
			catalogue_serialize::Distance distance_ser;
			for (auto& to : catalogue.GetAllStopsName()) {
				catalogue_serialize::Info info_distance;
				int distance = catalogue.GetDistanceStops(from, to);
				if (distance != 0) {
					info_distance.set_name(to);
					info_distance.set_distance(distance);
					*distance_ser.add_distance_to() = info_distance;
				}
			}
			if (distance_ser.distance_to_size() != 0) {
				distance_ser.set_name(from);
				*cat_ser.add_distances() = distance_ser;
			}
		}
		return cat_ser;
	}

	catalogue_serialize::Color SetColorSer(svg::Color type) {
		catalogue_serialize::Color color;
		if (std::holds_alternative<std::string>(type)) {
			color.add_color_name(std::get<std::string>(type));
		}
		if (std::holds_alternative<svg::Rgb>(type)) {
			color.add_color_name(std::to_string(std::get<svg::Rgb>(type).red));
			color.add_color_name(std::to_string(std::get<svg::Rgb>(type).green));
			color.add_color_name(std::to_string(std::get<svg::Rgb>(type).blue));
		}
		if (std::holds_alternative<svg::Rgba>(type)) {
			color.add_color_name(std::to_string(std::get<svg::Rgba>(type).red));
			color.add_color_name(std::to_string(std::get<svg::Rgba>(type).green));
			color.add_color_name(std::to_string(std::get<svg::Rgba>(type).blue));
			color.add_color_name(std::to_string(std::get<svg::Rgba>(type).opacity));
			color.set_opacity(std::get<svg::Rgba>(type).opacity);
		}
		return color;
	}

	catalogue_serialize::Renderer SerializeRenderer(const renderer::MapRender& render) {
		catalogue_serialize::Renderer rend_ser;

		rend_ser.set_width(render.width_);
		rend_ser.set_height(render.height_);
		rend_ser.set_padding(render.padding_);

		rend_ser.set_line_width(render.line_width_);
		rend_ser.set_stop_radius(render.stop_radius_);
		rend_ser.set_underlayer_width(render.underlayer_width_);
		rend_ser.set_bus_label_font_size(render.bus_label_font_size_);
		rend_ser.set_stop_label_font_size(render.stop_label_font_size_);

		rend_ser.add_bus_label_offset(render.bus_label_offset_.first);
		rend_ser.add_bus_label_offset(render.bus_label_offset_.second);

		rend_ser.add_stop_label_offset(render.stop_label_offset_.first);
		rend_ser.add_stop_label_offset(render.stop_label_offset_.second);

		*rend_ser.mutable_underlayer_color() = SetColorSer(render.underlayer_color_);

		for (auto& color : render.color_palette_) {
			*rend_ser.add_color_palette_() = SetColorSer(color);
		}
		return rend_ser;
	}

	catalogue_serialize::Edge SerializeEdge(const graph::Edge<double>& edge) {
		catalogue_serialize::Edge edge_ser;
		edge_ser.set_vertex_from(edge.from);
		edge_ser.set_vertex_to(edge.to);
		edge_ser.set_weight(edge.weight);
		return edge_ser;
	}

	catalogue_serialize::RouteInfo SerializeInfoRoute(const transport_router::RouteInfoCustom& data) {
		catalogue_serialize::RouteInfo route_ser;
		route_ser.set_count(data.count);
		route_ser.set_name(data.name);
		route_ser.set_time(data.time);
		if (data.type == transport_router::TypeRoute::WAIT) {
			route_ser.set_type("wait");
		}
		else {
			route_ser.set_type("ride");
		}
		return route_ser;
	}

	catalogue_serialize::Data SerializeRouterData(size_t line, std::optional<graph::Router<double>::RouteInternalData> data) {
		catalogue_serialize::Data data_ser;
		data_ser.set_line(line);
		if (data.has_value()) {
			catalogue_serialize::InternalData for_add;
			for_add.set_weight(data->weight);
			if (data->prev_edge.has_value()) {
				for_add.set_id(*data->prev_edge);
			}
			else {
				for_add.set_opt(false);
			}
			*data_ser.mutable_data() = for_add;
		}
		else {
			data_ser.set_nll(false);
		}
		return data_ser;;
	}

	catalogue_serialize::Router SerializeRouter(const transport_router::TransportRouter& transport_router) {
		catalogue_serialize::Router rout_ser;
		const auto& graph = transport_router.GetGraph();
		rout_ser.set_vertex_count(graph.GetVertexCount());
		for (size_t i = 0; i < graph.GetEdgeCount(); ++i) {
			const auto& edge = graph.GetEdge(i);
			*rout_ser.add_graph() = SerializeEdge(edge);
		}
		const auto& info = transport_router.GetInfo();
		for (const auto& data : info) {
			*rout_ser.add_info() = SerializeInfoRoute(data);
		}
		const auto& router = transport_router.GetRouter();
		size_t line = 0;
		for (const auto& within : router.GetInternalData()) {
			for (const auto& data : within) {
				*rout_ser.add_router() = SerializeRouterData(line, data);
			}
			++line;
		}
		return rout_ser;
	}

	void FullSerialize(const std::filesystem::path& path, const catalogue::TransportCatalogue& catalogue, 
                       const renderer::MapRender& render, const transport_router::TransportRouter& router) {
		std::ofstream out_file(path, std::ios::binary);
		catalogue_serialize::TransportCatalogue full_ser;

		*full_ser.mutable_catalogue() = SerializeCatalogue(catalogue);
		*full_ser.mutable_renderer() = SerializeRenderer(render);
		*full_ser.mutable_router() = SerializeRouter(router);

		full_ser.SerializeToOstream(&out_file);
	}

	domain::Stop DeserializeStop(catalogue_serialize::Stop ser_stop) {
		domain::Stop stop;
		stop.stop_name = ser_stop.name();
		stop.latitude = ser_stop.lati();
		stop.longitude = ser_stop.longi();
		stop.vertex_number = ser_stop.vertex();
		return stop;
	}

	domain::DistanceInfo DeserializeDistance(catalogue_serialize::Distance ser_distance) {
		domain::DistanceInfo distance;
		distance.stop_name = ser_distance.name();
		for (auto& info : ser_distance.distance_to()) {
			distance.distance_stops.push_back({ info.name(), info.distance() });
		}
		return distance;
	}

	void DeserializeCatalogue(const catalogue_serialize::Catalogue& cat_ser, catalogue::TransportCatalogue& catalogue) {
		for (auto& ser_stop : cat_ser.stops()) {
			catalogue.AddStop(DeserializeStop(ser_stop));
		}
		for (auto& buses : cat_ser.buses()) {
			std::vector<std::string> bu;
			for (auto& name : buses.names()) {
				bu.push_back(name);
			}
			catalogue.AddBus(bu, buses.is_round());
		}
		for (auto& distance : cat_ser.distances()) {
			catalogue.AddDistanceStops(DeserializeDistance(distance));
		}
	}
	svg::Color SetColorRender(const catalogue_serialize::Color& color) {
		if (color.color_name_size() == 1) {
			return { color.color_name()[0] };
		}
		if (color.color_name_size() == 3) {
			return svg::Rgb{ static_cast<uint8_t>(std::stoi(color.color_name()[0])),
				static_cast<uint8_t>(std::stoi(color.color_name()[1])),
				static_cast<uint8_t>(std::stoi(color.color_name()[2])) };
		}
		if (color.color_name_size() == 4) {
			return svg::Rgba{ static_cast<uint8_t>(std::stoi(color.color_name()[0])),
				static_cast<uint8_t>(std::stoi(color.color_name()[1])),
				static_cast<uint8_t>(std::stoi(color.color_name()[2])),
				color.opacity() };
		}
		return {};
	}

	void DeserializeRenderer(const catalogue_serialize::Renderer& rend_ser, renderer::MapRender& render) {
		render.width_ = rend_ser.width();
		render.height_ = rend_ser.height();
		render.padding_ = rend_ser.padding();

		render.line_width_ = rend_ser.line_width();
		render.stop_radius_ = rend_ser.stop_radius();

		render.underlayer_width_ = rend_ser.underlayer_width();
		render.bus_label_font_size_ = rend_ser.bus_label_font_size();
		render.stop_label_font_size_ = rend_ser.stop_label_font_size();

		render.bus_label_offset_ = { rend_ser.bus_label_offset()[0], rend_ser.bus_label_offset()[1] };
		render.stop_label_offset_ = { rend_ser.stop_label_offset()[0], rend_ser.stop_label_offset()[1] };

		render.underlayer_color_ = SetColorRender(rend_ser.underlayer_color());

		for (auto& color : rend_ser.color_palette_()) {
			render.color_palette_.push_back(SetColorRender(color));
		}
	}

	transport_router::RouteInfoCustom DeserializeData(catalogue_serialize::RouteInfo data) {
		transport_router::TypeRoute type;
		if (data.type() == "wait") {
			type = transport_router::TypeRoute::WAIT;
		}
		else {
			type = transport_router::TypeRoute::RIDE;
		}
		auto route_info = transport_router::RouteInfoCustom{ type, data.name(), data.count(), data.time() };
		return route_info;
	}

	std::optional<graph::Router<double>::RouteInternalData> DeserializeRouter(catalogue_serialize::Data ser_data) {
		std::optional<graph::Router<double>::RouteInternalData> route;
		if (ser_data.has_data()) {
			if (ser_data.data().test_case() == 3) {
				route.emplace(
                    graph::Router<double>::RouteInternalData{ ser_data.data().weight(), std::nullopt });
			}
			else {
				route.emplace(
                    graph::Router<double>::RouteInternalData{ ser_data.data().weight(), ser_data.data().id() });
			}
		}
		else {
			route = std::nullopt;
		}
		return route;
	}

	void DeserializeRouter(const catalogue_serialize::Router& rout_ser, transport_router::TransportRouter& router) {
		graph::DirectedWeightedGraph<double> graph(rout_ser.vertex_count());
		for (auto& edge : rout_ser.graph()) {
			graph.AddEdge({ edge.vertex_from(), edge.vertex_to(), edge.weight() });
		}
		std::vector<transport_router::RouteInfoCustom> info_catalogue;
		info_catalogue.reserve(rout_ser.info().size());
		for (auto& data : rout_ser.info()) {
			info_catalogue.push_back(DeserializeData(data));
		}
		graph::Router<double>::RoutesInternalData routes_data(rout_ser.vertex_count());
		for (auto& ser_data : rout_ser.router()) {
			routes_data[ser_data.line()].push_back(DeserializeRouter(ser_data));
		}
		router.SetGraph(graph);
		router.SetInfo(info_catalogue);
		router.SetRouterWithData(routes_data);
	}

	void FullDeserialize(const std::filesystem::path& path, catalogue::TransportCatalogue& catalogue,
		renderer::MapRender& render, transport_router::TransportRouter& router) {      
		std::ifstream in_file(path, std::ios::binary);
		catalogue_serialize::TransportCatalogue full_ser;
		full_ser.ParseFromIstream(&in_file);

		DeserializeCatalogue(full_ser.catalogue(), catalogue);
		DeserializeRenderer(full_ser.renderer(), render);
		DeserializeRouter(full_ser.router(), router);
	}
}