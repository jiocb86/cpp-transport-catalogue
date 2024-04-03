#pragma once

#include <iosfwd>
#include <string_view>
#include <iostream>
#include <iomanip>

#include "transport_catalogue.h"

namespace detail {
namespace outstat {
void ParseAndPrintStat(catalogue::TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output);
}
}
