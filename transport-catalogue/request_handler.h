#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

class RequestHandler {
public:
    explicit RequestHandler(const catalogue::TransportCatalogue& catalogue, const renderer::MapRenderer& renderer, const catalogue::Router& router)
        : catalogue_(catalogue)
        , renderer_(renderer)
        , router_(router)
    {
    }

    catalogue::BusInfo GetBusStat(const std::string_view& bus_number) const;
    const std::set<std::string_view> GetBusesByStop(std::string_view stop_name) const;
    bool IsBusNumber(const std::string_view bus_number) const;
    bool IsStopName(const std::string_view stop_name) const;    
    const std::optional<graph::Router<double>::RouteInfo> GetOptimalRoute(const std::string_view stop_from, const std::string_view stop_to) const;
    const graph::DirectedWeightedGraph<double>& GetRouterGraph() const;    
    
    svg::Document RenderMap() const;    

private:
    const catalogue::TransportCatalogue& catalogue_;
    const renderer::MapRenderer& renderer_;    
    const catalogue::Router& router_;    
};
