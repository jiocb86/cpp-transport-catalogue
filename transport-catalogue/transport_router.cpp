#include "transport_router.h"

namespace catalogue {

const graph::DirectedWeightedGraph<double>& Router::BuildGraph(const catalogue::TransportCatalogue& catalogue) {
	const auto& all_stops = catalogue.GetSortedAllStops();
	const auto& all_buses = catalogue.GetSortedAllBuses();
	graph::DirectedWeightedGraph<double> stops_graph(all_stops.size() * 2);
    std::map<std::string, graph::VertexId> stop_ids;
    graph::VertexId vertex_id = 0;

    for (const auto& [stop_name, stop_info] : all_stops) {
        stop_ids[stop_info->name] = vertex_id;
        stops_graph.AddEdge({
                stop_info->name,
                0,
                vertex_id,
                ++vertex_id,
                static_cast<double>(bus_wait_time_)
            });
        ++vertex_id;
    }
    stop_ids_ = std::move(stop_ids);

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
            
                stops_graph.AddEdge({bus_info->number,
                                  j - i,
                                  stop_ids_.at(stop_from->name) + 1,
                                  stop_ids_.at(stop_to->name),
                                  static_cast<double>(dist_sum) / (bus_velocity_ * (100.0 / 6.0))});

                if (!bus_info->is_circle) {
                    stops_graph.AddEdge({bus_info->number,
                                      j - i,
                                      stop_ids_.at(stop_to->name) + 1,
                                      stop_ids_.at(stop_from->name),
                                      static_cast<double>(dist_sum_inverse) / (bus_velocity_ * (100.0 / 6.0))});
                }
            }
        }
    }

    graph_ = std::move(stops_graph);
    router_ = std::make_unique<graph::Router<double>>(graph_);

    return graph_;
}

const std::optional<graph::Router<double>::RouteInfo> Router::FindRoute(const std::string_view stop_from, const std::string_view stop_to) const {
	return router_->BuildRoute(stop_ids_.at(std::string(stop_from)),stop_ids_.at(std::string(stop_to)));
}

const graph::DirectedWeightedGraph<double>& Router::GetGraph() const {
	return graph_;
}

}
