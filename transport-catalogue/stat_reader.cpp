#include "stat_reader.h"
#include "input_reader.h"

detail::reader::CommandDescription ParseCommands([[maybe_unused]]std::string_view line) {
    detail::reader::CommandDescription cd{};

    auto space_pos = line.find(' ');
    if (space_pos == line.npos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space == line.npos) {
        return {};
    }

    cd.command = std::string(line.substr(0, space_pos));
    cd.id = std::string(line.substr(not_space));
    return cd;
}

void detail::outstat::PrintStop(catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
    if (transport_catalogue.FindStop(request)) {
        catalogue::Stop stop;
        stop.name = request;
        transport_catalogue.AddBusesForStop(stop);
        std::set<std::string_view> buses = transport_catalogue.GetStopInfo(request);
        output << "Stop " << request << ": ";
        if (buses.empty()) {
            output << "no buses" << std::endl;
        } else {
            output << "buses";
            for (const auto& bus : buses) {
                output << " " << bus;
            }
            output << std::endl;
        }
    } else {
        output << "Stop " << request << ": not found" << std::endl;
    }    
}

void detail::outstat::PrintBus(catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {    
    if (transport_catalogue.FindBus(request)) {
        output << "Bus " << request << ": " << transport_catalogue.GetBusInfo(request).stops_count
        << " stops on route, " << transport_catalogue.GetBusInfo(request).unique_stops_count
        << " unique stops, " << std::setprecision(6) << transport_catalogue.GetBusInfo(request).route_length
        << " route length" << std::endl;
    } else {
        output << "Bus " << request << ": not found" << std::endl;
    }    
}

void detail::outstat::ParseAndPrintStat(catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
    detail::reader::CommandDescription cd = ParseCommands(request);
    if (cd.command == "Stop") {
        PrintStop(transport_catalogue, cd.id, output);
    }
    if (cd.command == "Bus") {
        PrintBus(transport_catalogue, cd.id, output);
    }    
}
