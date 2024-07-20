#include "map_renderer.h"

namespace renderer {

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Rgb ToRgb(int red, int green, int blue) {
        return {static_cast<uint8_t>(red),static_cast<uint8_t>(green), static_cast<uint8_t>(blue)};
    }

    svg::Rgba ToRgba(int red, int green, int blue, double opacity) {
        return {static_cast<uint8_t>(red), static_cast<uint8_t>(green), static_cast<uint8_t>(blue), opacity};
    }

    void AddBusText(svg::Document& doc, const renderer::MapRender& rend, SphereProjector proj, domain::Stop& stop, std::string name, svg::Color for_set) {
        doc.Add(svg::Text().SetData(name).
            SetPosition(proj({ stop.latitude,stop.longitude }))
            .SetOffset({rend.bus_label_offset_.first, rend.bus_label_offset_.second})
            .SetFontSize(rend.bus_label_font_size_)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetFillColor(rend.underlayer_color_)
            .SetStrokeColor(rend.underlayer_color_)
            .SetStrokeWidth(rend.underlayer_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
        doc.Add(svg::Text().SetData(name)
            .SetPosition(proj({ stop.latitude, stop.longitude }))
            .SetOffset({ rend.bus_label_offset_.first, rend.bus_label_offset_.second })
            .SetFontSize(rend.bus_label_font_size_)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetFillColor(for_set));
    }

    void AddStopText(svg::Document& doc, const renderer::MapRender& rend, SphereProjector proj, domain::Stop& stop, std::string name) {
        doc.Add(svg::Text().SetData(name).
            SetPosition(proj({ stop.latitude, stop.longitude }))
            .SetOffset({rend.stop_label_offset_.first, rend.stop_label_offset_.second})
            .SetFontSize(rend.stop_label_font_size_)
            .SetFontFamily("Verdana")
            .SetFillColor(rend.underlayer_color_)
            .SetStrokeColor(rend.underlayer_color_)
            .SetStrokeWidth(rend.underlayer_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
        doc.Add(svg::Text().SetData(name).
            SetPosition(proj({ stop.latitude, stop.longitude }))
            .SetOffset({ rend.stop_label_offset_.first, rend.stop_label_offset_.second })
            .SetFontSize(rend.stop_label_font_size_)
            .SetFontFamily("Verdana")
            .SetFillColor("black"));
    }

    svg::Polyline AddPolyline(const renderer::MapRender& rend, SphereProjector proj, int color_number_line,  std::vector<domain::Stop*> stops) {
        svg::Polyline line;
        line.SetStrokeColor(rend.color_palette_[color_number_line]).SetFillColor("none")
            .SetStrokeWidth(rend.line_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        for (auto& stop : stops) {
            line.AddPoint(proj({ stop->latitude, stop->longitude }));
        }
        return line;
    }

    void RenderMapWithCatalogue(const catalogue::TransportCatalogue& catal, const renderer::MapRender& rend, std::ostream& output) {
        svg::Document doc_for_render;
        std::vector<std::string> buses = catal.GetAllBusesName();
        std::sort(buses.begin(), buses.end());

        std::set<std::string> stops;
        std::vector<geo::Coordinates> coord;
        for (auto& bus : buses) {
            for (auto& stop : catal.FindBus(bus)->stops_on_bus) {
                coord.push_back({stop->latitude, stop->longitude});
                stops.insert(stop->stop_name);
            }
        }
        const SphereProjector proj{ coord.begin(), coord.end(), rend.width_, rend.height_, rend.padding_ };
        int size_color_pal = static_cast<int>(rend.color_palette_.size());
        int color_number_line = 0;
        for (auto& bus : buses) {
            doc_for_render.Add(AddPolyline(rend, proj, color_number_line, catal.FindBus(bus)->stops_on_bus));
            if (color_number_line < size_color_pal - 1) {
                ++color_number_line;
            }
            else {
                color_number_line = 0;
            }
        }
        int color_number_name = 0;
        for (auto& bus : buses) {
            if (!catal.FindBus(bus)) continue;
            int size_bus = static_cast<int>(catal.FindBus(bus)->stops_on_bus.size());
            domain::Stop* bus_begin = catal.FindBus(bus)->stops_on_bus[0];
            domain::Stop* bus_end = catal.FindBus(bus)->stops_on_bus[size_bus / 2];
            AddBusText(doc_for_render, rend, proj, *bus_begin, bus, rend.color_palette_[color_number_name]);
            if (!catal.FindBus(bus)->is_round && 
                (bus_end->stop_name != bus_begin->stop_name)) {
                AddBusText(doc_for_render,rend, proj, *bus_end, bus, rend.color_palette_[color_number_name]);
            }
            if (color_number_name < size_color_pal - 1) {
                ++color_number_name;
            }
            else {
                color_number_name = 0;
            }
        }
        for (auto& stop: stops) {
            domain::Stop* stop_render = catal.FindStop(stop);
            doc_for_render.Add(svg::Circle()
                .SetCenter(proj({stop_render->latitude, stop_render->longitude}))
                .SetRadius(rend.stop_radius_)
                .SetFillColor("white"));
        }
        for (auto& stop : stops) {
            domain::Stop* stop_render = catal.FindStop(stop);
            AddStopText(doc_for_render, rend, proj, *stop_render, stop);
        }

        doc_for_render.Render(output);

    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }
}