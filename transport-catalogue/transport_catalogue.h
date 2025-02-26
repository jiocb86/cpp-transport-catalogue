#pragma once

#include <algorithm>
#include <deque>
#include <string>
#include <string_view>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>

#include "geo.h"
#include "domain.h"

namespace catalogue {

class TransportCatalogue {
    public:
    struct StopPairHasher {
        
        //хэшер для пары указателей на остановки         
        std::size_t operator()(const std::pair<const Stop*, const Stop*>& p) const {                   
        std::size_t hash1 = std::hash<const void*>{}(p.first);
        std::size_t hash2 = std::hash<const void*>{}(p.second);
            
        //комбинируем хеши для обоих указателей            
        return hash1 + hash2 * 37;                                                                     
    }
};    
        void AddStop(const Stop& stop);
        const Stop* FindStop(const std::string_view stop) const;            
        void AddBus(const Bus& bus);                                        
        const Bus* FindBus(const std::string_view bus) const;               
    
        //получить информацию о маршруте    
        const BusInfo GetBusInfo(const std::string_view bus) const;         
    
        //поиск автобусов проходящих через остановку 
        const std::unordered_set<const Bus*> FindBusesForStop(const std::string_view stop_name) const; 
    
        //получить информацию об остановке
        const std::set<std::string_view> GetStopInfo(const std::string_view stop_name) const;          
    
        //задать дистанцию между остановками
        void SetDistance(const Stop* from, const Stop* to, const int distance);                        
    
        //получить дистанцию между остановками
        int GetDistance(const Stop* from, const Stop* to) const;         
    
        //получить отсортированные маршруты
        const std::map<std::string_view, const Bus*> GetSortedAllBuses() const;
    
        //получить отсортированные остановки
        const std::map<std::string_view, const Stop*> GetSortedAllStops() const;
    
    private:
        std::deque<Stop> stops_;                                                                       
        std::deque<Bus> buses_;                                                                        
    
        //индекс остановок(хеш - таблица)
        std::unordered_map<std::string_view, const Bus*> busname_to_bus_;                              
    
        //индекс маршрутов(хеш - таблица)
        std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;                           
        
        //автобусы проходящие через остановку
        std::unordered_map<const Stop*, std::unordered_set<const Bus*>> buses_for_stop_;               
    
        //расстояние между остановками
        std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopPairHasher> distances_;       
};
} // namespace catalogue
