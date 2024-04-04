#include "transport_catalogue.h"

void catalogue::TransportCatalogue::AddStop(const Stop& stop){
    stops_.push_back(stop);
    stopname_to_stop_.insert({std::move(stops_.back().name), &stops_.back()});
}

void catalogue::TransportCatalogue::AddBus(const Bus& bus){
    buses_.push_back(bus);
    busname_to_bus_.insert({std::move(buses_.back().number), &buses_.back()});
}

const catalogue::Stop* catalogue::TransportCatalogue::FindStop(const std::string_view stop) const {
    if (stopname_to_stop_.count(stop)) {
        return stopname_to_stop_.at(stop);
    }
    return nullptr;
}

const catalogue::Bus* catalogue::TransportCatalogue::FindBus(const std::string_view bus) const {
    if (busname_to_bus_.count(bus)) {
        return busname_to_bus_.at(bus);
    }
    return nullptr;
}

const std::unordered_set<const catalogue::Bus*> catalogue::TransportCatalogue::FindBusesForStop(const std::string_view stop_name) const {
    const Stop* stop_ptr = FindStop(stop_name);    
    if (busname_to_stop_.count(stop_ptr)) {
        return busname_to_stop_.at(stop_ptr);
    }
    return {};
}

const catalogue::BusInfo catalogue::TransportCatalogue::GetBusInfo(const std::string_view bus_number) const {
    catalogue::BusInfo bus_info{};
    const catalogue::Bus* bus = FindBus(bus_number);
    if (bus) {
        {
            bus_info.stops_count = bus->route.size();
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
            double route_length = 0.0;
            const auto& route = FindBus(bus->number)->route;
            for (int i = 1; i < route.size(); ++i) {
                const catalogue::Stop* current_stop = route[i - 1];
                const catalogue::Stop* next_stop = route[i];
                route_length += ComputeDistance(current_stop->coordinates, next_stop->coordinates);
            }
            bus_info.route_length = route_length;           
        }
    }
    return bus_info;
}

void catalogue::TransportCatalogue::AddBusesForStop(const Stop& stop) {
    const Stop* stop_ptr = FindStop(stop.name);
    for (const Bus& bus : buses_) {
        const auto& route = FindBus(bus.number)->route;            
        for (const auto& stop : route) {
            if (stop_ptr == stop){            
                busname_to_stop_[stop_ptr].insert(FindBus(bus.number));
            }
        }
    }
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
