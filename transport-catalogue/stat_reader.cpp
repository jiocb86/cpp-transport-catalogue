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

void detail::outstat::ParseAndPrintStat(catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
    detail::reader::CommandDescription cd = ParseCommands(request);
    if (cd.command == "Stop") {
        if (transport_catalogue.FindStop(cd.id)) {
            transport_catalogue.BusesForStop(cd.id);
            output << "Stop " << cd.id << ": ";
            std::set<std::string_view> buses = transport_catalogue.GetStopInfo(cd.id);
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
            output << "Stop " << cd.id << ": not found" << std::endl;
        }
    }
    if (cd.command == "Bus") {
        if (transport_catalogue.FindBus(cd.id)) {
            output << request << ": " << transport_catalogue.GetBusInfo(cd.id).stops_count
                << " stops on route, " << transport_catalogue.GetBusInfo(cd.id).unique_stops_count
                << " unique stops, " << std::setprecision(6) << transport_catalogue.GetBusInfo(cd.id).route_length
                << " route length" << std::endl;
        } else {
            output << request << ": not found" << std::endl;
        }
    }
}
