#include "transport_catalogue.h"

void catalogue::TransportCatalogue::AddStop(const Stop& stop){
    stops_.push_back(stop);
    stopname_to_stop_.insert({std::move(stops_.back().name), &stops_.back()});
}

void catalogue::TransportCatalogue::AddBus(const Bus& bus){
    buses_.push_back(bus);
    busname_to_bus_.insert({std::move(buses_.back().number), &buses_.back()});
    for (const Stop* stop : bus.route) {
        if (stopname_to_stop_.count(stop->name)) {
            buses_for_stop_[stopname_to_stop_[stop->name]].insert(&buses_.back());
        }
    }    
}

const catalogue::Stop* catalogue::TransportCatalogue::FindStop(const std::string_view stop) const {
    auto it = stopname_to_stop_.find(stop);
    if (it != stopname_to_stop_.end()) {
        return it->second;
    }
    return nullptr;
}

const catalogue::Bus* catalogue::TransportCatalogue::FindBus(const std::string_view bus) const {
    auto it = busname_to_bus_.find(bus);
    if (it != busname_to_bus_.end()) {
        return it->second;
    }
    return nullptr;
}

const std::unordered_set<const catalogue::Bus*> catalogue::TransportCatalogue::FindBusesForStop(const std::string_view stop_name) const {
    const Stop* stop_ptr = FindStop(stop_name);    
    auto it = buses_for_stop_.find(stop_ptr);
    if (it != buses_for_stop_.end()) {
        return it->second;
    }
    return {};
}

const catalogue::BusInfo catalogue::TransportCatalogue::GetBusInfo(const std::string_view bus_number) const {
    catalogue::BusInfo bus_info{};
    const catalogue::Bus* bus = FindBus(bus_number);
    if (bus) {
        {
            if (bus->is_circle) {
                bus_info.stops_count = bus->route.size();
            } else {
                bus_info.stops_count = bus->route.size() * 2 - 1;            
            }
        }
        {
            std::unordered_set<const Stop*> unique_stops;
            const auto& route = FindBus(bus->number)->route;
            for (const auto& stop : route) {
                unique_stops.insert(stop);    
            }
            bus_info.unique_stops_count = unique_stops.size();
        }
        {
            int dist_length = 0;
            double geo_length = 0.0;            
            const auto& route = FindBus(bus->number)->route;   
            for (std::size_t i = 1; i < route.size(); ++i) {
                const catalogue::Stop* from = route[i - 1];
                const catalogue::Stop* to = route[i];
                if (bus->is_circle) {
                    dist_length += GetDistance(from, to);
                    geo_length += geo::ComputeDistance(from->coordinates, to->coordinates);
                } else {
                    dist_length += GetDistance(from, to) + GetDistance(to, from);
                    geo_length += geo::ComputeDistance(from->coordinates, to->coordinates) * 2;
                }
            }
            bus_info.dist_length = dist_length;
            bus_info.geo_length = geo_length;
        }       
    }
    return bus_info;
}

const std::set<std::string_view> catalogue::TransportCatalogue::GetStopInfo(const std::string_view stop_name) const {
    std::set<std::string_view> buses;  
    const Stop* stop_ptr = FindStop(stop_name);
    if (stop_ptr) {
        for (const auto& bus : FindBusesForStop(stop_name)) {
            if (bus) {
                buses.insert(std::move(bus->number));  
            }
        }
    }
    return buses;
}

void catalogue::TransportCatalogue::SetDistance(const Stop* from, const Stop* to, const int dist) {
    distances_[{from, to}] = dist;
}

int catalogue::TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
    auto it = distances_.find({ from, to });
    if (it != distances_.end()) {
        return it->second;
    }
    it = distances_.find({ to, from });
    if (it != distances_.end()) {
        return it->second;
    }
    return 0;
}

const std::map<std::string_view, const catalogue::Bus*> catalogue::TransportCatalogue::GetSortedAllBuses() const {
    std::map<std::string_view, const Bus*> result;
    for (const auto& bus : busname_to_bus_) {
        result.emplace(bus);
    }
    return result;
}
