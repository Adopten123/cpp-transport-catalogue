#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"
#include "input_reader.h"

namespace transport {
    namespace reader {
        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output);
    }
}

