#pragma once

#include <algorithm>
#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>

#include "geo.h"

namespace catalogue {
struct Stop {
    std::string name;
    geo::Coordinates coordinates;
};

struct Bus {
    std::string number;
    std::vector<const Stop*> route;
};

struct BusInfo {
    int stops_count;
    int unique_stops_count;
    double geo_length;
    int dist_length;
};

class TransportCatalogue {
    public:
    struct StopPairHasher {
        std::size_t operator()(const std::pair<const Stop*, const Stop*>& p) const {                   //хэшер для пары указателей на остановки 
        std::size_t hash1 = std::hash<const void*>{}(p.first);
        std::size_t hash2 = std::hash<const void*>{}(p.second);
        return hash1 + hash2 * 37;                                                                     // Комбинируем хеши для обоих указателей
    }
};    
        void AddStop(const Stop& stop);                                                                //добавить остановку в базу
        const Stop* FindStop(const std::string_view stop) const;                                       //поиск остановки по имени 
        void AddBus(const Bus& bus);                                                                   //добавить маршрут в базу
        const Bus* FindBus(const std::string_view bus) const;                                          //поиск маршрута по имени
        const BusInfo GetBusInfo(const std::string_view bus) const;                                    //получить информацию о маршруте
        void AddBusesForStop(const Stop& stop);                                                        //добавить автобусы проходящие через остановку
        const std::unordered_set<const Bus*> FindBusesForStop(const std::string_view stop_name) const; //поиск автобусов проходящих через остановку 
        const std::set<std::string_view> GetStopInfo(const std::string_view stop_name) const;          //получить информацию об остановке
        void SetDistance(const Stop* from, const Stop* to, const int distance);                        //задать дистанцию между остановками
        int GetDistance(const Stop* from, const Stop* to) const;                                       //получить дистанцию между остановками
    private:
        std::deque<Stop> stops_;                                                                       //остановки
        std::deque<Bus> buses_;                                                                        //маршруты
        std::unordered_map<std::string_view, const Bus*> busname_to_bus_;                              //индекс остановок(хеш - таблица)
        std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;                           //индекс маршрутов(хеш - таблица)
        std::unordered_map<const Stop*, std::unordered_set<const Bus*>> buses_for_stop_;               //автобусы проходящие через остановку
        std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopPairHasher> distances_;       //расстояние между остановками
};
}
