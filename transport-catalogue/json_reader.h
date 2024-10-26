#pragma once

#include <variant>
#include <sstream>

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace reader{

struct CommandDescription {
    // Определяет, задана ли команда (поле command непустое)
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }
    
    CommandDescription() = default;

    CommandDescription(const std::string_view& command, const std::string_view& id, const std::variant<std::string_view, geo::Coordinates, std::vector<std::string_view>>& description, const std::variant<std::string_view, bool, std::vector<std::pair<std::string_view, int>>>& details = {})
        : command(command), id(id), description(description), details(details) {
    }

    CommandDescription(const std::string_view& command, const std::string_view& id, const std::variant<std::string_view, geo::Coordinates, std::vector<std::string_view>>& description, const std::map<std::string_view, int>& sv_map)
        : command(command), id(id), description(description) {
        std::vector<std::pair<std::string_view, int>> vect;
        for (const auto& [key, value] : sv_map) {
            vect.emplace_back(std::string(key), value);
        }
        details = vect;
    }

    std::string_view command;      // Название команды
    std::string_view id;           // id маршрута или остановки
    // Параметры команды
    std::variant<std::string_view, geo::Coordinates, std::vector<std::string_view>> description;
    // Данные комманды
    std::variant<std::string_view, bool, std::vector<std::pair<std::string_view, int>>> details;
};
    
class JsonReader {
public:
    JsonReader(std::istream& input) : input_(json::Load(input)) {
    }
    
    const json::Node& GetBaseRequests() const;
    const json::Node& GetStatRequests() const;
    const json::Node& GetRenderSettings() const;    
    const json::Node& GetRoutingSettings() const;    
    
    void ParseBaseRequests();
    void ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const;    
    void ApplyCommands(catalogue::TransportCatalogue& catalogue) const;
    svg::Color ParseColor(const json::Node& color_node) const;
    renderer::MapRenderer ParseRenderSettings(const json::Dict& request_map) const;
    catalogue::TransportRouter::Settings FillRoutingSettings(const json::Node& settings) const;    
    
    const json::Node PrintRoute(const json::Dict& request_map, RequestHandler& rh) const;
    const json::Node PrintStop(const json::Dict& request_map, RequestHandler& rh) const;
    const json::Node PrintMap(const json::Dict& request_map, RequestHandler& rh) const;
    const json::Node PrintRouting(const json::Dict& request_map, RequestHandler& rh) const;    
    
private:
    json::Document input_;
    json::Node dummy_ = nullptr;
    std::vector<CommandDescription> commands_;
    std::tuple<std::string_view, std::vector<const catalogue::Stop*>, bool> FillRoute(const json::Dict& request_map, catalogue::TransportCatalogue& catalogue) const;    
};
} // namespace reader
