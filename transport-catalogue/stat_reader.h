#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"
#include "input_reader.h"

namespace transport {
    namespace reader {
        std::string BusesToString(const std::set<Bus*, BusPtrHasher>& buses);

        void ParseAndPrintBusStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output);

        void ParseAndPrintStopStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output);

        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output);

        void ShowCatalogue(TransportCatalogue& catalogue);
    }


}

