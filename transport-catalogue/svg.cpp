#include "svg.h"

namespace svg {

using namespace std::literals;
    
inline void PrintColor(std::ostream& out, Rgb& rgb) {
    out << "rgb("sv << static_cast<short>(rgb.red) << ","sv
                    << static_cast<short>(rgb.green) << ","sv 
                    << static_cast<short>(rgb.blue) << ")"sv;
}
    
inline void PrintColor(std::ostream& out, Rgba& rgba) {
    out << "rgba("sv << static_cast<short>(rgba.red) << ","sv 
                     << static_cast<short>(rgba.green) << ","sv 
                     << static_cast<short>(rgba.blue) << ","sv 
                     << (rgba.opacity) << ")"sv;
}
    
inline void PrintColor(std::ostream& out, std::monostate) {
    out << "none"sv;
}
 
inline void PrintColor(std::ostream& out, std::string& color) {
    out << color;
}
    
std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit([&out](auto value) {
            PrintColor(out, value);
    }, color);
    
    return out;
}     

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << "/>"sv;    
}

// ---------- Text ------------------

void TextData(std::ostream& out, std::string_view sv) {
    for (char ch : sv) {
        switch (ch) {
            case '"':
                out << "&quot;"sv;
                break;
            case '<':
                out << "&lt;"sv;
                break;
            case '>':
                out << "&gt;"sv;
                break;
            case '&':
                out << "&amp;"sv;
                break;
            case '\'':
                out << "&apos;"sv;
                break;
            default:
                out << ch;
        }
    }
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
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;

    out << "<text ";
    RenderAttrs(context.out);
    out << "x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << font_size_ << "\" "sv;
 
    if (!font_family_.empty()) {
        out << "font-family=\""sv << font_family_ << "\" "sv;
    }
 
    if (!font_weight_.empty()) {
        out << "font-weight=\""sv << font_weight_ << "\" "sv;
    }
    out << '>';
    TextData(out, data_);
    out << "</text>"sv;
}
    
// ---------- Polyline ------------------    
    
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool first = true;
    for (const Point& point : points_) {
        if (first) {
            first = false;
        } else {
            out << ' ';
        }
        out << point.x << ',' << point.y;
    }
    out << "\" "sv;
    RenderAttrs(context.out);    
    out << "/>"sv;
}    

// ---------- Document ------------------
    
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    RenderContext context{out, 2, 2};
    for (const auto& obj : objects_) {
        obj->Render(context);
    }
    out << "</svg>"sv;
}
    
}  // namespace svg
