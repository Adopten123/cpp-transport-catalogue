#include "stat_reader.h"

#include <iostream>
#include <set>

namespace transport {
    namespace reader {
        std::string BusesToString(const std::set<Bus*, BusPtrHasher>& buses) {
            std::string result = "";

            for (const Bus* bus : buses) {
                using namespace std;
                result += bus->name_;
                result += " "s;
            }
            return result;
        }

        void ParseAndPrintBusStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output) {
            using namespace std;

            string_view bus_name = request.substr(request.find(' ') + 1);
            const Bus* bus = tansport_catalogue.GetBus(bus_name);

            if (bus) {
                BusInfo businfo = tansport_catalogue.GetBusInfo(bus);

                output << "Bus "s << bus_name << ": "s << to_string(businfo.stops_count_) << " stops on route, "s
                    << to_string(businfo.unique_stops_count_) << " unique stops, "s << to_string(businfo.route_length_) << " route length"s << endl;
            }
            else {
                output << "Bus "s << bus_name << ": not found"s << endl;
            }

        }

        void ParseAndPrintStopStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output) {
            using namespace std;

            std::string_view stop_name = request.substr(request.find(' ') + 1);
            const Stop* stop = tansport_catalogue.GetStop(stop_name);

            if (stop) {
                if (tansport_catalogue.GetBusesByStop(stop_name).size() == 0) {
                    output << "Stop "s << stop_name << ": no buses" << endl;
                }
                else {
                    output << "Stop "s << stop_name << ": buses " << BusesToString(tansport_catalogue.GetBusesByStop(stop_name)) << endl;
                }
            }
            else {
                output << "Stop "s << stop_name << ": not found" << endl;
            }

        }

        void ParseAndPrintStat(const TransportCatalogue& tansport_catalogue, std::string_view request,
            std::ostream& output) {
            using namespace std;

            string_view command_name = request.substr(0, request.find(' '));

            if (command_name == "Bus"sv) {
                ParseAndPrintBusStat(tansport_catalogue, request, output);
            }
            else if (command_name == "Stop"sv) {
                ParseAndPrintStopStat(tansport_catalogue, request, output);
            }
        }

        void ShowCatalogue(std::istream& input, std::ostream& output, const TransportCatalogue& catalogue) {
            int stat_request_count;
            input >> stat_request_count >> std::ws;
            for (int i = 0; i < stat_request_count; ++i) {
                std::string line;
                getline(input, line);
                transport::reader::ParseAndPrintStat(catalogue, line, output);
            }
        }
    }
}

