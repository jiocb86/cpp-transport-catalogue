#include <iostream>

#include "json_reader.h"
#include "request_handler.h"

using namespace reader;
using namespace catalogue;

int main() {
    TransportCatalogue catalogue;
    JsonReader json_doc(std::cin);
    // Ввод данных
    json_doc.ParseBaseRequests();
    json_doc.ApplyCommands(catalogue);
    // Вывод данных
    const auto& stat_requests = json_doc.GetStatRequests();    
    const auto& render_settings = json_doc.GetRenderSettings().AsDict();
    const auto& renderer = json_doc.ParseRenderSettings(render_settings);
    const auto& routing_settings = json_doc.FillRoutingSettings(json_doc.GetRoutingSettings());
    const catalogue::TransportRouter router(routing_settings, catalogue);  

    RequestHandler rh(catalogue, renderer, router);
    json_doc.ProcessRequests(stat_requests, rh);    
    return 0;
}
