#include "json_reader.h"

using namespace std;

namespace reader {
    JsonReader::JsonReader(std::istream& input) : json_doc_(json::Load(input))
    {
    }

    void JsonReader::FillDataFromJson() {
        FillCatalogue(json_doc_.GetRoot().AsDict().at("base_requests").AsArray());
        FillMapRenderer(json_doc_.GetRoot().AsDict().at("render_settings").AsDict());

        const json::Dict& dict = json_doc_.GetRoot().AsDict().at("routing_settings").AsDict();
        router_.BuildGraph(dict.at("bus_velocity").AsDouble(), dict.at("bus_wait_time").AsInt(), catalogue_);
        router_.SetRouter();
    }

    void JsonReader::SerializeData() const {
        const std::filesystem::path path = json_doc_.GetRoot().AsDict().at("serialization_settings").AsDict().at("file").AsString();
        serialization::FullSerialize(path, catalogue_, render_, router_);
    }

    void JsonReader::DeserializeData() {
        const std::filesystem::path path = json_doc_.GetRoot().AsDict().at("serialization_settings").AsDict().at("file").AsString();
        serialization::FullDeserialize(path, catalogue_, render_, router_);
    }

    void JsonReader::PrintRequest(std::ostream& output) const {
        PrintRequestJSON(json_doc_.GetRoot().AsDict().at("stat_requests").AsArray(), output);
    }

    void JsonReader::AddStopFromJson(const json::Node& stop) {
        catalogue_.AddStop({
                    stop.AsDict().at("name").AsString() ,
                    stop.AsDict().at("latitude").AsDouble() ,
                    stop.AsDict().at("longitude").AsDouble()
            });
    }

    void JsonReader::AddDistanceFromJson(const json::Node& distance) {
        std::vector<std::pair<std::string, int>> stops_to_stop;
        for (auto& [stop, distance] : distance.AsDict().at("road_distances").AsDict()) {
            stops_to_stop.push_back({ stop, distance.AsInt() });
        }
        catalogue_.AddDistanceStops({
            distance.AsDict().at("name").AsString(),
            std::move(stops_to_stop) });
    }

    void JsonReader::AddBusFromJson(const json::Node& bus) {
        std::vector<std::string> bus_info;
        bus_info.push_back(bus.AsDict().at("name").AsString());

        if (bus.AsDict().at("is_roundtrip").AsBool()) {
            for (auto& stop : bus.AsDict().at("stops").AsArray()) {
                bus_info.push_back(stop.AsString());
            }
        }
        else {
            std::vector<std::string> back_stops;
            for (auto& stop : bus.AsDict().at("stops").AsArray()) {
                bus_info.push_back(stop.AsString());
                back_stops.push_back(stop.AsString());
            }
            for (auto it = back_stops.rbegin() + 1; it < back_stops.rend(); it++) {
                bus_info.push_back(*it);
            }
        }
        catalogue_.AddBus(std::move(bus_info), bus.AsDict().at("is_roundtrip").AsBool());
    }

    void JsonReader::FillCatalogue(const json::Array& base_req) {
        for (auto& add_stop : base_req) {
            if (add_stop.AsDict().at("type").AsString() == "Stop") {
                AddStopFromJson(add_stop);
            }
        }
        for (auto& add_distance_stop : base_req) {
            if (add_distance_stop.AsDict().at("type").AsString() == "Stop") {
                AddDistanceFromJson(add_distance_stop);
            }
        }
        for (auto& add_bus : base_req) {
            if (add_bus.AsDict().at("type").AsString() == "Bus") {
                AddBusFromJson(add_bus);
            }
        }
    }

    svg::Color JsonReader::FillColor(const json::Node& color) {
        if (color.IsString()) {
            return color.AsString();
        }
        else {
            if (color.AsArray().size() == 4) {
                return renderer::ToRgba(
                    color.AsArray()[0].AsInt(),
                    color.AsArray()[1].AsInt(),
                    color.AsArray()[2].AsInt(),
                    color.AsArray()[3].AsDouble());
            }
            else {
                return renderer::ToRgb(
                    color.AsArray()[0].AsInt(),
                    color.AsArray()[1].AsInt(),
                    color.AsArray()[2].AsInt());
            }
        }
    }

    void JsonReader::FillMapRenderer(const json::Dict& stat_req) {
        render_.width_ = stat_req.at("width").AsDouble();
        render_.height_ = stat_req.at("height").AsDouble();
        render_.padding_ = stat_req.at("padding").AsDouble();
        render_.line_width_ = stat_req.at("line_width").AsDouble();
        render_.stop_radius_ = stat_req.at("stop_radius").AsDouble();
        render_.underlayer_width_ = stat_req.at("underlayer_width").AsDouble();
        render_.bus_label_font_size_ = stat_req.at("bus_label_font_size").AsInt();
        render_.stop_label_font_size_ = stat_req.at("stop_label_font_size").AsInt();
        render_.bus_label_offset_ = { stat_req.at("bus_label_offset").AsArray()[0].AsDouble(),
                                     stat_req.at("bus_label_offset").AsArray()[1].AsDouble() };
        render_.stop_label_offset_ = { stat_req.at("stop_label_offset").AsArray()[0].AsDouble(),
                                        stat_req.at("stop_label_offset").AsArray()[1].AsDouble() };
        render_.underlayer_color_ = FillColor(stat_req.at("underlayer_color"));
        for (auto& color : stat_req.at("color_palette").AsArray()) {
            render_.color_palette_.push_back(FillColor(color));
        }
    }

    json::Dict JsonReader::CreateStopRequest(const json::Node& stop_req) const {
        json::Builder stop_info_build;
        std::string stop_name = stop_req.AsDict().at("name").AsString();
        if (!catalogue_.FindStop(stop_name)) {
            stop_info_build
                .StartDict()
                .Key("request_id"s).Value(stop_req.AsDict().at("id").GetValue())
                .Key("error_message"s).Value("not found"s)
                .EndDict();
        }
        else {
            std::set<std::string> buses_stop = catalogue_.GetInfoStop(stop_name);
            json::Array buses;
            for (auto& bus : buses_stop) {
                buses.push_back(bus);
            }
            stop_info_build
                .StartDict()
                .Key("buses"s).Value(buses)
                .Key("request_id"s).Value(stop_req.AsDict().at("id").GetValue())
                .EndDict();
        }
        return stop_info_build.Build().AsDict();
    }

    json::Dict JsonReader::CreateBusRequest(const json::Node& bus_req) const {
        json::Builder bus_info_build;
        std::string bus_name = bus_req.AsDict().at("name").AsString();
        if (!catalogue_.FindBus(bus_name)) {
            bus_info_build
                .StartDict()
                .Key("request_id"s).Value(bus_req.AsDict().at("id").GetValue())
                .Key("error_message"s).Value("not found"s)
                .EndDict();
        }
        else {
            domain::BusInfo bus = catalogue_.GetInfoBus(bus_name);
            bus_info_build
                .StartDict()
                .Key("curvature"s).Value(bus.curvature)
                .Key("request_id"s).Value(bus_req.AsDict().at("id").GetValue())
                .Key("route_length"s).Value(bus.route_length)
                .Key("stop_count"s).Value(bus.stops_on_route)
                .Key("unique_stop_count"s).Value(bus.unique_stops)
                .EndDict();
        }
        return bus_info_build.Build().AsDict();
    }

    json::Dict JsonReader::CreateMapRequest(const json::Node& map_req) const {
        json::Builder map_info_build;
        std::ostringstream renderer_out;
        renderer::RenderMapWithCatalogue(catalogue_, render_, renderer_out);
        map_info_build
            .StartDict()
            .Key("map"s).Value(renderer_out.str())
            .Key("request_id"s).Value(map_req.AsDict().at("id").GetValue())
            .EndDict();
        return map_info_build.Build().AsDict();
    }

    json::Dict JsonReader::CreateRouteRequest( const json::Node& route_req) const {
        json::Builder route_info_build;
        std::string route_from = route_req.AsDict().at("from").AsString();
        std::string route_to = route_req.AsDict().at("to").AsString();
        std::optional<std::vector<transport_router::RouteInfoCustom>> result = router_.BuildOptimalRoute(catalogue_.FindStop(route_from)->vertex_number * 2,
            catalogue_.FindStop(route_to)->vertex_number * 2);
        if (result) {
            route_info_build.StartDict().Key("items").StartArray();
            for (int i = 1; i < result->size(); i++) {
                if ((*result)[i].type == transport_router::TypeRoute::WAIT) {
                    route_info_build.StartDict()
                        .Key("stop_name"s).Value((*result)[i].name)
                        .Key("time"s).Value((*result)[i].time)
                        .Key("type"s).Value("Wait"s)
                        .EndDict();
                }
                else {
                    route_info_build.StartDict()
                        .Key("bus"s).Value((*result)[i].name)
                        .Key("span_count"s).Value((*result)[i].count)
                        .Key("time"s).Value((*result)[i].time)
                        .Key("type"s).Value("Bus"s)
                        .EndDict();
                }
            }
            route_info_build.EndArray()
                .Key("request_id"s).Value(route_req.AsDict().at("id").GetValue())
                .Key("total_time"s).Value((*result)[0].time)
                .EndDict();
        }
        else {
            route_info_build
                .StartDict()
                .Key("error_message"s).Value("not found"s)
                .Key("request_id"s).Value(route_req.AsDict().at("id").GetValue())
                .EndDict();
        }

        return route_info_build.Build().AsDict();
    }

    void JsonReader::PrintRequestJSON(const json::Array& stat_req, std::ostream& out) const {
        json::Array requests_print;
        json::Builder req;
        req.StartArray();
        for (auto& request : stat_req) {
            if (request.AsDict().at("type").AsString() == "Stop") {
                req.Value(CreateStopRequest(request));
            }
            else if (request.AsDict().at("type").AsString() == "Bus") {
                req.Value(CreateBusRequest(request));
            }
            else if (request.AsDict().at("type").AsString() == "Route") {
                req.Value(CreateRouteRequest(request));
            }
            else {
                req.Value(CreateMapRequest(request));
            }
        }
        req.EndArray();
        json::Print(json::Document{ req.Build() }, out);
    }
}