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
    const auto& render_settings = json_doc.GetRenderSettings().AsMap();
    const auto& renderer = json_doc.ParseRenderSettings(render_settings);
    RequestHandler rh(catalogue, renderer);
    json_doc.ProcessRequests(stat_requests, rh);    
    return 0;
}
