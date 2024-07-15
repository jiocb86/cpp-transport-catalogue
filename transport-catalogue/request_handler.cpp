#include "request_handler.h"

bool RequestHandler::IsBusNumber(const std::string_view bus_number) const {
    return catalogue_.FindBus(bus_number);
}

bool RequestHandler::IsStopName(const std::string_view stop_name) const {
    return catalogue_.FindStop(stop_name);
}

catalogue::BusInfo RequestHandler::GetBusStat(const std::string_view& bus_number) const {
    return catalogue_.GetBusInfo(bus_number);
}

const std::set<std::string_view> RequestHandler::GetBusesByStop(std::string_view stop_name) const {
    return catalogue_.GetStopInfo(stop_name);
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.GetSVG(catalogue_.GetSortedAllBuses());
}
