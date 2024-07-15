#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <cmath>
#include <variant>

namespace svg {
    
class Rgb {
public:
    Rgb() = default;
    Rgb(uint8_t red, uint8_t green, uint8_t blue) : red(red)
                                                  , green(green)
                                                  , blue(blue){};
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};
 
inline void PrintColor(std::ostream& out, Rgb& rgb);
 
class Rgba {
public:
    Rgba() = default;
    Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity) : red(red)
                                                                   , green(green)
                                                                   , blue(blue)
                                                                   , opacity(opacity) {};
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};  
    
inline void PrintColor(std::ostream& out, Rgba& rgba);
 
using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{"none"};  
    
inline void PrintColor(std::ostream& out, std::monostate);
inline void PrintColor(std::ostream& out, std::string& color);
std::ostream& operator<<(std::ostream& out, const Color& color); 

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};    
   
inline std::ostream &operator<<(std::ostream &out, StrokeLineCap stroke_line_cap) {
    using namespace std::literals;
    if (stroke_line_cap == StrokeLineCap::BUTT) {
        out << "butt"sv;
    } else if (stroke_line_cap == StrokeLineCap::ROUND) {
        out << "round"sv;
    } else if (stroke_line_cap == StrokeLineCap::SQUARE) {
        out << "square"sv;
    }
    return out;
}
 
inline std::ostream &operator<<(std::ostream &out, StrokeLineJoin stroke_line_join) {
    using namespace std::literals;
    if (stroke_line_join == StrokeLineJoin::ARCS) {
        out << "arcs"sv;
    } else if (stroke_line_join == StrokeLineJoin::BEVEL) {
        out << "bevel"sv;
    } else if (stroke_line_join == StrokeLineJoin::MITER) {
        out << "miter"sv;
    } else if (stroke_line_join == StrokeLineJoin::MITER_CLIP) {
        out << "miter-clip"sv;
    } else if (stroke_line_join == StrokeLineJoin::ROUND) {
        out << "round"sv;
    }
    return out;
}

template<typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
 
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }
 
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_line_cap_ = line_cap;
        return AsOwner();
    }
 
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_line_join_ = line_join;
        return AsOwner();
    } 

protected:
    ~PathProps() = default;
    
    // Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke
    void RenderAttrs(std::ostream &out) const {
        using namespace std::literals;
 
        if (fill_color_ != std::nullopt) {
            out << "fill=\""sv << fill_color_.value() << "\" "sv;
        }
        
        if (stroke_color_ != std::nullopt) {
            out << "stroke=\""sv << stroke_color_.value() << "\" "sv;
        }
        
        if (stroke_width_ != std::nullopt) {
            out << "stroke-width=\""sv << stroke_width_.value() << "\" "sv;
        }
        
        if (stroke_line_cap_ != std::nullopt) {
            out << "stroke-linecap=\""sv << stroke_line_cap_.value() << "\" "sv;
        }
        
        if (stroke_line_join_ != std::nullopt) {
            out << "stroke-linejoin=\""sv << stroke_line_join_.value() << "\" "sv;
        }
    }
    
private: 
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps    
        return static_cast<Owner&>(*this);
    }
    
    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
};
    
struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
// Наследованием от PathProps<Circle> мы «сообщаем» родителю,
// что владельцем свойств является класс Circle    
class Circle : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
    */
private:
    void RenderObject(const RenderContext& context) const override;
    std::vector<Point> points_;    
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    // Прочие данные и методы, необходимые для реализации элемента <text>
private:
    void RenderObject(const RenderContext& context) const override;
    Point pos_;
    Point offset_;
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
};

class ObjectContainer {
public:
    template <typename ObjectType>
    void Add(ObjectType object) {
        AddPtr(std::make_unique<ObjectType>(std::move(object)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

protected:
    ~ObjectContainer() = default;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
};

class Document : public ObjectContainer {
public:
    /*
     Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
     Пример использования:
     Document doc;
     doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
    */
 
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;
    
    // Прочие методы и данные, необходимые для реализации класса Document
private:
    std::vector<std::unique_ptr<Object>> objects_;    
};

}  // namespace svg

namespace shapes {
 
class Triangle : public svg::Drawable {

public:
    Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
            : p1_(p1)
            , p2_(p2)
            , p3_(p3) {
    }
 
    // Реализует метод Draw интерфейса svg::Drawable
    void Draw(svg::ObjectContainer& container) const override {
        container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
    }
 
private:
    svg::Point p1_, p2_, p3_;
 };
 
class Star : public svg::Drawable {

public:
    svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
        using namespace svg;
        svg::Polyline polyline;        
        polyline.SetFillColor("red");
        polyline.SetStrokeColor("black");        
        for (int i = 0; i <= num_rays; ++i) {
            double angle = 2 * M_PI * (i % num_rays) / num_rays;
            polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
            if (i == num_rays) {
                break;
            }
            angle += M_PI / num_rays;
            polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
        }
        return polyline;
    }
    Star(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
        polyline_ = CreateStar(center, outer_rad, inner_rad, num_rays);
    }
 
    void Draw(svg::ObjectContainer& container) const override {
        container.Add(polyline_);
    }
    
private:    
    svg::Polyline polyline_;
};
 
class Snowman : public svg::Drawable {

public:
    Snowman(svg::Point head_center, double head_radius) : head_center_(head_center), head_radius_(head_radius)
    {}
 
    void Draw(svg::ObjectContainer& container) const override {
        using namespace svg;
 
        Circle head, middle, down;
        head.SetFillColor("rgb(240,240,240)");
        head.SetStrokeColor("black");        
        head.SetCenter(head_center_).SetRadius(head_radius_);
        middle.SetFillColor("rgb(240,240,240)");
        middle.SetStrokeColor("black"); 
        middle.SetCenter({head_center_.x, head_center_.y + head_radius_ * 2}).SetRadius(head_radius_ * 1.5);
        down.SetFillColor("rgb(240,240,240)");
        down.SetStrokeColor("black");        
        down.SetCenter({head_center_.x, head_center_.y + head_radius_ * 5}).SetRadius(head_radius_ * 2);
        container.Add(down);
        container.Add(middle);
        container.Add(head);
    }
    
private:        
    svg::Point head_center_;
    double head_radius_;
};
 
} // namespace shapes
