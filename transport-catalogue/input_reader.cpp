#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace detail{
/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
catalogue::geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

std::vector<std::pair<std::string_view, int>> ParseDistances(std::string_view line) {
    std::vector<std::pair<std::string_view, int>> result;
    std::vector<std::string_view> stops_dist = Split(line, ',');
    for (auto stop_dist : stops_dist) {
        auto m_to_pos = stop_dist.find("m to");
        if (m_to_pos < stop_dist.length()) {
            auto dist = std::stoi(std::string(Trim(stop_dist.substr(0, m_to_pos))));
            auto stop = Trim(stop_dist.substr(m_to_pos + 4));
            result.push_back(make_pair(std::move(stop), dist));
        }
    }
    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

reader::CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}
}
    
void detail::reader::InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}
    
#include <iostream>    
void detail::reader::InputReader::ApplyCommands([[maybe_unused]] catalogue::TransportCatalogue& catalogue) const {
    for (const auto& cd : commands_) {
        if (cd.command == "Stop") {
            catalogue::Stop stop;
            stop.name = cd.id;
            stop.coordinates = ParseCoordinates(cd.description);
            catalogue.AddStop(stop);
        }
    }
    for (const auto& cd : commands_) {
        if (cd.command == "Stop") {
            const catalogue::Stop* stop_from = catalogue.FindStop(cd.id);
            std::vector<std::pair<std::string_view, int>> dist = ParseDistances(cd.description);
            for (std::vector<std::pair<std::string_view,int> >::iterator it = dist.begin(); it != dist.end(); ++it) {
                const catalogue::Stop* stop_to = catalogue.FindStop(it->first);
                catalogue.SetDistance(stop_from, stop_to, it->second);
            }    
        }
    }
    for (const auto& cd : commands_) {    
        if (cd.command == "Bus") {
            catalogue::Bus bus;
            bus.number = cd.id;
            std::vector<std::string_view> route = ParseRoute(cd.description);
            for (const auto& stop : route) {
                const catalogue::Stop* stop_ptr = catalogue.FindStop(stop);
                bus.route.push_back(stop_ptr);
            }
            catalogue.AddBus(bus);
        }
    }
    for (const auto& cd : commands_) {
        if (cd.command == "Stop") {    
            catalogue::Stop stop;
            stop.name = cd.id;
            catalogue.AddBusesForStop(stop); 
        }
    }
}
