#include "json_reader.h"

namespace reader {

const json::Node& JsonReader::GetBaseRequests() const {
    if (!input_.GetRoot().AsMap().count("base_requests")) return dummy_;
    return input_.GetRoot().AsMap().at("base_requests");
}
    
const json::Node& JsonReader::GetStatRequests() const {
    if (!input_.GetRoot().AsMap().count("stat_requests")) return dummy_;
    return input_.GetRoot().AsMap().at("stat_requests");
}

const json::Node& JsonReader::GetRenderSettings() const {
    if (!input_.GetRoot().AsMap().count("render_settings")) return dummy_;
    return input_.GetRoot().AsMap().at("render_settings");
}
    
void JsonReader::ParseBaseRequests() {    
    const json::Array& arr = GetBaseRequests().AsArray();    
    for (const auto& request : arr) {
        const auto& request_map = request.AsMap();
        std::string_view command = request_map.at("type").AsString();
        std::string_view id = request_map.at("name").AsString();
        if (command == "Stop") {
            double lat = request_map.at("latitude").AsDouble();
            double lng = request_map.at("longitude").AsDouble();
            geo::Coordinates coordinates{lat, lng};
            std::vector<std::pair<std::string_view, int>> road_distances;
            auto& distances = request_map.at("road_distances").AsMap();
            for (auto& [name, distance] : distances) {
                road_distances.emplace_back(name, distance.AsInt());
            }            
            commands_.emplace_back(command, id, coordinates, road_distances);            
        } else if (command == "Bus") {
            std::vector<std::string_view> route;
            for (const auto& stop : request_map.at("stops").AsArray()) {
                route.emplace_back(stop.AsString());
            }
            bool is_roundtrip = request_map.at("is_roundtrip").AsBool();            
            commands_.emplace_back(command, id, route, is_roundtrip);
        }
    }
}
    
void JsonReader::ApplyCommands([[maybe_unused]]catalogue::TransportCatalogue& catalogue) const {
    for (const auto& cd : commands_) {
        if (cd.command == "Stop") {
            catalogue::Stop stop;
            stop.name = cd.id;
            stop.coordinates = std::get<geo::Coordinates>(cd.description);
            catalogue.AddStop(stop);
        }
    }
    for (const auto& cd : commands_) {
        if (cd.command == "Stop") {
            const catalogue::Stop* stop_from = catalogue.FindStop(cd.id);
            std::vector<std::pair<std::string_view, int>> dist = std::get<std::vector<std::pair<std::string_view, int>>>(cd.details);
            for (std::vector<std::pair<std::string_view,int> >::iterator it = dist.begin(); it != dist.end(); ++it) {
                const catalogue::Stop* stop_to = catalogue.FindStop(it->first);
                catalogue.SetDistance(stop_from, stop_to, it->second);
            }    
        }
    }
    for (const auto& cd : commands_) {    
        if (cd.command == "Bus") {
            catalogue::Bus bus;
            bus.number = cd.id;
            std::vector<std::string_view> route = std::get<std::vector<std::string_view>>(cd.description);
            for (const auto& stop : route) {
                const catalogue::Stop* stop_ptr = catalogue.FindStop(stop);
                bus.route.push_back(stop_ptr);
            }
            bus.is_circle = std::get<bool>(cd.details);
            catalogue.AddBus(bus);
        }
    }
}
    
renderer::MapRenderer JsonReader::ParseRenderSettings(const json::Dict& request_map) const {
    renderer::RenderSettings render_settings;
    render_settings.width = request_map.at("width").AsDouble();
    render_settings.height = request_map.at("height").AsDouble();
    render_settings.padding = request_map.at("padding").AsDouble();
    render_settings.stop_radius = request_map.at("stop_radius").AsDouble();
    render_settings.line_width = request_map.at("line_width").AsDouble();
    render_settings.bus_label_font_size = request_map.at("bus_label_font_size").AsInt();
    const json::Array& bus_label_offset = request_map.at("bus_label_offset").AsArray();
    render_settings.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };
    render_settings.stop_label_font_size = request_map.at("stop_label_font_size").AsInt();
    const json::Array& stop_label_offset = request_map.at("stop_label_offset").AsArray();
    render_settings.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };
    
    if (request_map.at("underlayer_color").IsString()) {
        render_settings.underlayer_color = request_map.at("underlayer_color").AsString();
    } else if (request_map.at("underlayer_color").IsArray()) {
        const json::Array& underlayer_color = request_map.at("underlayer_color").AsArray();
        if (underlayer_color.size() == 3) {
            render_settings.underlayer_color = svg::Rgb(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt());
        } else if (underlayer_color.size() == 4) {
            render_settings.underlayer_color = svg::Rgba(underlayer_color[0].AsInt(), underlayer_color[1].AsInt(), underlayer_color[2].AsInt(), underlayer_color[3].AsDouble());
        } else {
            throw std::logic_error("wrong underlayer colortype");
        }
    } else {
        throw std::logic_error("wrong underlayer color");
    }
    
    render_settings.underlayer_width = request_map.at("underlayer_width").AsDouble();
    
    const json::Array& color_palette = request_map.at("color_palette").AsArray();
    for (const auto& color_element : color_palette) {
        if (color_element.IsString()) {
            render_settings.color_palette.push_back(color_element.AsString());
        } else if (color_element.IsArray()) {
            const json::Array& color_type = color_element.AsArray();
            if (color_type.size() == 3) {
                render_settings.color_palette.emplace_back(svg::Rgb(color_type[0].AsInt(), color_type[1].AsInt(), color_type[2].AsInt()));
            } else if (color_type.size() == 4) {
                render_settings.color_palette.push_back(svg::Rgba(color_type[0].AsInt(), color_type[1].AsInt(), color_type[2].AsInt(), color_type[3].AsDouble()));
            } else {
                throw std::logic_error("wrong color_palette type");
            }
        } else {
            throw std::logic_error("wrong color_palette");
        }
    }
    return render_settings;
}
    
const json::Node JsonReader::PrintRoute(const json::Dict& request_map, RequestHandler& rh) const {
    json::Dict result;
    const std::string& route_number = request_map.at("name").AsString();
    result["request_id"] = request_map.at("id").AsInt();
    if (!rh.IsBusNumber(route_number)) {
        result["error_message"] = json::Node{ static_cast<std::string>("not found") };
    }
    else {
        result["curvature"] = rh.GetBusStat(route_number).dist_length/rh.GetBusStat(route_number).geo_length;
        result["route_length"] = rh.GetBusStat(route_number).dist_length;
        result["stop_count"] = static_cast<int>(rh.GetBusStat(route_number).stops_count);
        result["unique_stop_count"] = static_cast<int>(rh.GetBusStat(route_number).unique_stops_count);
    }

    return json::Node{ result };
}

const json::Node JsonReader::PrintStop(const json::Dict& request_map, RequestHandler& rh) const {
    json::Dict result;
    const std::string& stop_name = request_map.at("name").AsString();
    result["request_id"] = request_map.at("id").AsInt();
    if (!rh.IsStopName(stop_name)) {
        result["error_message"] = json::Node{ static_cast<std::string>("not found") };
    } else {
        json::Array buses;
        for (const auto& bus : rh.GetBusesByStop(stop_name)) {
            buses.push_back(json::Node(std::string(bus)));
        }
        result["buses"] = buses;
    }

    return json::Node{ result };
}
    
const json::Node JsonReader::PrintMap(const json::Dict& request_map, RequestHandler& rh) const {
    json::Dict result;
    result["request_id"] = request_map.at("id").AsInt();
    std::ostringstream strm;
    svg::Document map = rh.RenderMap();
    map.Render(strm);
    result["map"] = strm.str();

    return json::Node{ result };   
}
    
void JsonReader::ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const {
    json::Array result;
    for (auto& request : stat_requests.AsArray()) {
        const auto& request_map = request.AsMap();
        const auto& type = request_map.at("type").AsString();
        if (type == "Stop") {
            result.emplace_back(PrintStop(request_map, rh).AsMap());
        }
        if (type == "Bus") {
            result.emplace_back(PrintRoute(request_map, rh).AsMap());
        }
        if (type == "Map") {
            result.emplace_back(PrintMap(request_map, rh).AsMap());
        }
    }

    json::Print(json::Document{ result }, std::cout);
}    

} // namespace reader
