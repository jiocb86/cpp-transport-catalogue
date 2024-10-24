#include "json_reader.h"
#include "json_builder.h"

namespace reader {

const json::Node& JsonReader::GetBaseRequests() const {
    auto it = input_.GetRoot().AsDict().find("base_requests");
    if (it == input_.GetRoot().AsDict().end()) {
        return dummy_;
    }
    return it->second;
}
    
const json::Node& JsonReader::GetStatRequests() const {
    auto it = input_.GetRoot().AsDict().find("stat_requests");
    if (it == input_.GetRoot().AsDict().end()) {
        return dummy_;
    } 
    return it->second;
}

const json::Node& JsonReader::GetRenderSettings() const {
    auto it = input_.GetRoot().AsDict().find("render_settings");
    if (it == input_.GetRoot().AsDict().end()) {
        return dummy_;
    }
    return it->second;
}
    
const json::Node& JsonReader::GetRoutingSettings() const {
    auto it = input_.GetRoot().AsDict().find("routing_settings");    
    if (it == input_.GetRoot().AsDict().end()) {
        return dummy_;
    }
    return it->second;
}
    
void JsonReader::ParseBaseRequests() {    
    const json::Array& arr = GetBaseRequests().AsArray();    
    for (const auto& request : arr) {
        const auto& request_map = request.AsDict();
        std::string_view command = request_map.at("type").AsString();
        std::string_view id = request_map.at("name").AsString();
        if (command == "Stop") {
            double lat = request_map.at("latitude").AsDouble();
            double lng = request_map.at("longitude").AsDouble();
            geo::Coordinates coordinates{lat, lng};
            std::vector<std::pair<std::string_view, int>> road_distances;
            auto& distances = request_map.at("road_distances").AsDict();
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
    
svg::Color JsonReader::ParseColor(const json::Node& color_node) const {
    if (color_node.IsString()) {
        return color_node.AsString();
    } else if (color_node.IsArray()) {
        const json::Array& color_array = color_node.AsArray();
        if (color_array.size() == 3) {
            return svg::Rgb(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt());
        } else if (color_array.size() == 4) {
            return svg::Rgba(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble());
        } else {
            throw std::logic_error("wrong underlayer colortype");
        }
    } else {
        throw std::logic_error("wrong underlayer color");
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
    
    render_settings.underlayer_color = ParseColor(request_map.at("underlayer_color"));
    
    render_settings.underlayer_width = request_map.at("underlayer_width").AsDouble();
    
    const json::Array& color_palette = request_map.at("color_palette").AsArray();
    for (const auto& color_element : color_palette) {
        render_settings.color_palette.push_back(ParseColor(color_element));
    }    

    return render_settings;
}
    
const json::Node JsonReader::PrintRoute(const json::Dict& request_map, RequestHandler& rh) const {
    json::Node result;
    const std::string& route_number = request_map.at("name").AsString();
    const int id = request_map.at("id").AsInt();
    if (!rh.IsBusNumber(route_number)) {
        result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(id)
                        .Key("error_message").Value("not found")
                    .EndDict()
                .Build();
    } else {
        result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(id)
                        .Key("curvature").Value(rh.GetBusStat(route_number).dist_length/rh.GetBusStat(route_number).geo_length)
                        .Key("route_length").Value(rh.GetBusStat(route_number).dist_length)
                        .Key("stop_count").Value(static_cast<int>(rh.GetBusStat(route_number).stops_count))
                        .Key("unique_stop_count").Value(static_cast<int>(rh.GetBusStat(route_number).unique_stops_count))
                    .EndDict()
                .Build();
    }
    return result;
}

const json::Node JsonReader::PrintStop(const json::Dict& request_map, RequestHandler& rh) const {
    json::Node result;
    const std::string& stop_name = request_map.at("name").AsString();
    const int id = request_map.at("id").AsInt();
    if (!rh.IsStopName(stop_name)) {
        result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(id)
                        .Key("error_message").Value("not found")
                    .EndDict()
                .Build();
    } else {
        json::Array buses;
        for (const auto& bus : rh.GetBusesByStop(stop_name)) {
            buses.push_back(json::Node(std::string(bus)));
        }
        result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(id)
                        .Key("buses").Value(buses)
                    .EndDict()
                .Build();
    }
    return result;
}

const json::Node JsonReader::PrintMap(const json::Dict& request_map, RequestHandler& rh) const {
    json::Node result;
    const int id = request_map.at("id").AsInt();
    std::ostringstream strm;
    svg::Document map = rh.RenderMap();
    map.Render(strm);
    result = json::Builder{}
                .StartDict()
                    .Key("request_id").Value(id)
                    .Key("map").Value(strm.str())
                .EndDict()
            .Build();
    return result;    
}
    
void JsonReader::ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const {
    json::Array result;
    for (auto& request : stat_requests.AsArray()) {
        const auto& request_map = request.AsDict();
        const auto& type = request_map.at("type").AsString();
        if (type == "Stop") {
            result.emplace_back(PrintStop(request_map, rh).AsDict());
        }
        if (type == "Bus") {
            result.emplace_back(PrintRoute(request_map, rh).AsDict());
        }
        if (type == "Map") {
            result.emplace_back(PrintMap(request_map, rh).AsDict());
        }
        if (type == "Route") {
            result.emplace_back(PrintRouting(request_map, rh).AsDict());        
        }
    }

    json::Print(json::Document{ result }, std::cout);
}
    
std::tuple<std::string_view, std::vector<const catalogue::Stop*>, bool> JsonReader::FillRoute(const json::Dict& request_map, catalogue::TransportCatalogue& catalogue) const {
    std::string_view bus_number = request_map.at("name").AsString();
    std::vector<const catalogue::Stop*> stops;
    for (auto& stop : request_map.at("stops").AsArray()) {
        stops.push_back(catalogue.FindStop(stop.AsString()));
    }
    bool circular_route = request_map.at("is_roundtrip").AsBool();

    return std::make_tuple(bus_number, stops, circular_route);
}
    
catalogue::Router JsonReader::FillRoutingSettings(const json::Node& settings) const {
    catalogue::Router routing_settings;
    return catalogue::Router{ settings.AsDict().at("bus_wait_time").AsInt(), settings.AsDict().at("bus_velocity").AsDouble() };
}    
    
const json::Node JsonReader::PrintRouting(const json::Dict& request_map, RequestHandler& rh) const {
    json::Node result;
    const int id = request_map.at("id").AsInt();
    const std::string_view stop_from = request_map.at("from").AsString();
    const std::string_view stop_to = request_map.at("to").AsString();
    const auto& routing = rh.GetOptimalRoute(stop_from, stop_to);
    
    if (!routing) {
        result = json::Builder{}
            .StartDict()
                .Key("request_id").Value(id)
                .Key("error_message").Value("not found")
            .EndDict()
        .Build();
    }
    else {
        json::Array items;
        double total_time = 0.0;
        items.reserve(routing.value().edges.size());
        for (auto& edge_id : routing.value().edges) {
            const graph::Edge<double> edge = rh.GetRouterGraph().GetEdge(edge_id);
            if (edge.quality == 0) {
                items.emplace_back(json::Node(json::Builder{}
                    .StartDict()
                        .Key("stop_name").Value(edge.name)
                        .Key("time").Value(edge.weight)
                        .Key("type").Value("Wait")
                    .EndDict()
                .Build()));

                total_time += edge.weight;
            }
            else {
                items.emplace_back(json::Node(json::Builder{}
                    .StartDict()
                        .Key("bus").Value(edge.name)
                        .Key("span_count").Value(static_cast<int>(edge.quality))
                        .Key("time").Value(edge.weight)
                        .Key("type").Value("Bus")
                    .EndDict()
                .Build()));

                total_time += edge.weight;
            }
        }

        result = json::Builder{}
            .StartDict()
                .Key("request_id").Value(id)
                .Key("total_time").Value(total_time)
                .Key("items").Value(items)
            .EndDict()
        .Build();
    }

    return result;
}    

} // namespace reader
