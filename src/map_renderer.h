#pragma once

#include "geo.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

namespace renderer {

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) 
        {
            if (points_begin == points_end) {
                return;
            }
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }
            if (width_zoom && height_zoom) {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                zoom_coeff_ = *height_zoom;
            }
        }
        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    struct MapRender {
        double width_;
        double height_;
        double padding_;
        double line_width_;
        double stop_radius_;
        double underlayer_width_;
        int bus_label_font_size_;
        int stop_label_font_size_;
        svg::Color underlayer_color_;
        std::vector<svg::Color> color_palette_;
        std::pair<double, double> bus_label_offset_;
        std::pair<double, double> stop_label_offset_;
    };

    svg::Rgb ToRgb(int red, int green, int blue);
    svg::Rgba ToRgba(int red, int green, int blue, double opacity);

    void AddBusText(svg::Document& doc, const renderer::MapRender& rend, SphereProjector proj, domain::Stop& stop, std::string name, svg::Color for_set);
    void AddStopText(svg::Document& doc, const renderer::MapRender& rend, SphereProjector proj, domain::Stop& stop, std::string name);
    svg::Polyline AddPolyline(const renderer::MapRender& rend, SphereProjector proj, int color_number_line, std::vector<domain::Stop*> stops);
    void RenderMapWithCatalogue(const catalogue::TransportCatalogue& catal, const renderer::MapRender& rend, std::ostream& output);
}