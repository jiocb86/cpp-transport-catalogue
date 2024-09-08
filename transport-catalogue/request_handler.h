#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

class RequestHandler {
public:
    explicit RequestHandler(const catalogue::TransportCatalogue& catalogue, const renderer::MapRenderer& renderer)
        : catalogue_(catalogue)
        , renderer_(renderer)
    {
    }

    //void ParseStatRequests(const json::Node& stat_requests) const;
    
    catalogue::BusInfo GetBusStat(const std::string_view& bus_number) const;
    const std::set<std::string_view> GetBusesByStop(std::string_view stop_name) const;
    bool IsBusNumber(const std::string_view bus_number) const;
    bool IsStopName(const std::string_view stop_name) const;    
    
    svg::Document RenderMap() const;    

private:
    const catalogue::TransportCatalogue& catalogue_;
    const renderer::MapRenderer& renderer_;    
};
