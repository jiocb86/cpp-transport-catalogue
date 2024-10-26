#include "transport_router.h"

namespace catalogue {
    //добавляет остановки в граф
    void TransportRouter::AddStopsToGraph(graph::DirectedWeightedGraph<double>& stops_graph, std::map<std::string, graph::VertexId>& stop_ids, graph::VertexId& vertex_id, const catalogue::TransportCatalogue& catalogue) {
        const auto& all_stops = catalogue.GetSortedAllStops(); 
        
        for (const auto& [stop_name, stop_info] : all_stops) {
            stop_ids[stop_info->name] = vertex_id;
            stops_graph.AddEdge({
                stop_info->name,
                0,
                vertex_id,
                ++vertex_id,
                static_cast<double>(GetBusWaitTime())
            });
            ++vertex_id;
        }
    }
    
    //добавляет автобусы в граф
    void TransportRouter::AddBusesToGraph(graph::DirectedWeightedGraph<double>& stops_graph, const catalogue::TransportCatalogue& catalogue) {
        const auto& all_buses = catalogue.GetSortedAllBuses();  
        
        for (const auto& item : all_buses) {
            const auto& bus_info = item.second;
            const auto& route = bus_info->route;
            size_t route_len = route.size();

            for (size_t i = 0; i < route_len; ++i) {
                for (size_t j = i + 1; j < route_len; ++j) {
                    const catalogue::Stop* stop_from = route[i];
                    const catalogue::Stop* stop_to = route[j];
                    int dist_sum = 0;
                    int dist_sum_inverse = 0;

                    for (size_t k = i + 1; k <= j; ++k) {
                        dist_sum += catalogue.GetDistance(route[k - 1], route[k]);
                        dist_sum_inverse += catalogue.GetDistance(route[k], route[k - 1]);
                    }

                    stops_graph.AddEdge({
                        bus_info->number,
                        j - i,
                        stop_ids_.at(stop_from->name) + 1,
                        stop_ids_.at(stop_to->name),
                        static_cast<double>(dist_sum) / (GetBusVelocity() * (100.0 / 6.0))
                    });

                    if (!bus_info->is_circle) {
                        stops_graph.AddEdge({
                            bus_info->number,
                            j - i,
                            stop_ids_.at(stop_to->name) + 1,
                            stop_ids_.at(stop_from->name),
                            static_cast<double>(dist_sum_inverse) / (GetBusVelocity() * (100.0 / 6.0))
                        });
                    }
                }
            }
        }
    }    

void TransportRouter::BuildGraph(const catalogue::TransportCatalogue& catalogue) {
    const auto& all_stops = catalogue.GetSortedAllStops();     
	graph::DirectedWeightedGraph<double> stops_graph(all_stops.size() * 2);
    std::map<std::string, graph::VertexId> stop_ids;
    graph::VertexId vertex_id = 0;

    AddStopsToGraph(stops_graph, stop_ids, vertex_id, catalogue);
    
    stop_ids_ = std::move(stop_ids);
    
    AddBusesToGraph(stops_graph, catalogue);
    
    graph_ = std::move(stops_graph);
    router_ = std::make_unique<graph::Router<double>>(graph_);
}

const TransportRouter::RouteItems TransportRouter::GetRouteInfo(const std::string_view stop_from, const std::string_view stop_to) const {
    auto router_info = router_->BuildRoute(stop_ids_.at(std::string(stop_from)), stop_ids_.at(std::string(stop_to)));
    graph::Router<double>::RouteInfo items_info;
    
    if (router_info) {
        items_info.weight = router_info.value().weight;
        for (const auto& edge : router_info.value().edges) {
                items_info.edges.push_back(edge);
            }
        return RouteItems{items_info, &graph_};
    } else {
        return RouteItems{std::nullopt, nullptr};
    }

    return RouteItems{items_info, &graph_};
}
    
}
