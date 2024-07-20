#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap line_cap) {
        if (line_cap == StrokeLineCap::BUTT) {
            out << "butt"sv;
        }
        if (line_cap == StrokeLineCap::ROUND) {
            out << "round"sv;
        }
        if (line_cap == StrokeLineCap::SQUARE) {
            out << "square"sv;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin line_join) {
        switch (line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs";
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel";
            break;
        case StrokeLineJoin::MITER:
            out << "miter";
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip";
            break;
        case StrokeLineJoin::ROUND:
            out << "round";
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const Color color) {
        std::ostringstream strm;
        std::visit(ColorPrinter{ strm }, color);
        out << strm.str();
        return out;
    }


    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);

        context.out << std::endl;
    }

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << " />"sv;
    }

    Polyline& Polyline::AddPoint(Point point) {
        std::ostringstream x_strs;
        std::ostringstream y_strs;
        x_strs << point.x;
        y_strs << point.y;
        if (polyline_.empty()) {
            polyline_ += (x_strs.str() + ',' + y_strs.str());
        }
        else {
            polyline_ += (' ' + x_strs.str() + ',' + y_strs.str());
        }
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv << polyline_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    } 

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\""sv;
        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv;
        for (char symbol : data_) {
            switch (symbol) {
            case '\"':
                out << "&quot;";
                break;
            case '\'':
                out << "&apos;";
                break;
            case '<':
                out << "&lt;";
                break;
            case '>':
                out << "&gt;";
                break;
            case '&':
                out << "&amp;";
                break;
            default:
                out << symbol;
                break;
            }
        }
        out << "</text>"sv;
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext ctx(out, 2, 2);
        for (auto& obj : objects_) {
            obj->Render(ctx);
        }
        out << "</svg>"sv;
    }

}  // namespace svg