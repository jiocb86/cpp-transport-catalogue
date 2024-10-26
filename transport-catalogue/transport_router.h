#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>

namespace catalogue {

class TransportRouter {
public:

    struct Settings {
        int bus_wait_time_ = 0;
        double bus_velocity_ = 0.0;
    };

    using RouteInfo = graph::Router<double>::RouteInfo;
    
    struct RouteItems {
        std::optional<RouteInfo> route_info_;
        const graph::DirectedWeightedGraph<double>* route_graph_;
    };

    TransportRouter() = default;

    TransportRouter(const Settings& settings, const catalogue::TransportCatalogue& catalogue)
        : settings_(settings) {
        BuildGraph(catalogue);
    }

    const RouteItems GetRouteInfo(const std::string_view stop_from, const std::string_view stop_to) const;
    
protected:    
    
    int GetBusWaitTime() const {
        return settings_.bus_wait_time_;
    }

    double GetBusVelocity() const {
        return settings_.bus_velocity_;
    }     

    void BuildGraph(const catalogue::TransportCatalogue& catalogue);
    
private:
    Settings settings_;

    graph::DirectedWeightedGraph<double> graph_;        
    std::map<std::string, graph::VertexId> stop_ids_;
    std::unique_ptr<graph::Router<double>> router_;        

    void AddStopsToGraph(graph::DirectedWeightedGraph<double>& stops_graph, std::map<std::string, graph::VertexId>& stop_ids, graph::VertexId& vertex_id, const catalogue::TransportCatalogue& catalogue);
        
    void AddBusesToGraph(graph::DirectedWeightedGraph<double>& stops_graph, const catalogue::TransportCatalogue& catalogue);
};
    
}
