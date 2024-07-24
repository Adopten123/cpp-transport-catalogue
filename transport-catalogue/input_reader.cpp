#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <sstream>

namespace transport {
    namespace reader {
        transport::geo::Coordinates ParseCoordinates(std::string_view str) {
            static const double nan = std::nan("");

            auto not_space = str.find_first_not_of(' ');
            auto comma = str.find(',');

            if (comma == str.npos) {
                return { nan, nan };
            }

            auto not_space2 = str.find_first_not_of(' ', comma + 1);

            double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
            double lng = std::stod(std::string(str.substr(not_space2)));

            return { lat, lng };
        }

        std::string_view Trim(std::string_view string) {
            const auto start = string.find_first_not_of(' ');
            if (start == string.npos) {
                return {};
            }
            return string.substr(start, string.find_last_not_of(' ') + 1 - start);
        }

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

        std::vector<std::string_view> ParseRoute(std::string_view route) {
            if (route.find('>') != route.npos) {
                return Split(route, '>');
            }

            auto stops = Split(route, '-');
            std::vector<std::string_view> results(stops.begin(), stops.end());
            results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

            return results;
        }

        CommandDescription ParseCommandDescription(std::string_view line) {
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

            return { std::string(line.substr(0, space_pos)),
                    std::string(line.substr(not_space, colon_pos - not_space)),
                    std::string(line.substr(colon_pos + 1)) };
        }

        std::unordered_map<std::string, size_t> ParseDistances(std::string_view str) {
            std::unordered_map<std::string, size_t> distances;
            auto distances_parts = Split(str, ',');

            for (const auto& part : distances_parts) {
                auto to_pos = part.find("m to ");
                if (to_pos != part.npos) {
                    int distance = std::stoi(std::string(part.substr(0, to_pos)));
                    std::string stop_name = std::string(Trim(part.substr(to_pos + 5)));
                    distances[stop_name] = distance;
                }
            }

            return distances;
        }

        void InputReader::ParseLine(std::string_view line) {
            auto command_description = ParseCommandDescription(line);
            if (command_description) {
                commands_.push_back(std::move(command_description));
            }
        }

        void InputReader::FillDistances(TransportCatalogue& catalogue) const {
            for (const CommandDescription& command : commands_) {
                if (command.command == "Stop") {
                    
                    size_t pos = command.description.find(',', command.description.find(',') + 1);

                    auto distances_str = command.description.substr(command.description.find(',', pos) + 1);
                    auto distances = ParseDistances(distances_str);

                    for (const auto& [neighbor_stop, distance] : distances) {
                        catalogue.SetDistance(const_cast<transport::Stop*>(catalogue.GetStop(command.id)),
                            const_cast<transport::Stop*>(catalogue.GetStop(neighbor_stop)), distance);
                    }
                }
            }
        }

        void InputReader::ApplyCommands(TransportCatalogue& catalogue) const {
            std::vector<CommandDescription> bus_commands;

            for (const CommandDescription& command : commands_) {
                if (command.command == "Stop") {
                    
                    size_t pos = command.description.find(',', command.description.find(',')+1);

                    auto coords_str = command.description.substr(0, pos);
                    auto coordinates = ParseCoordinates(coords_str);

                    catalogue.AddStop({ command.id, coordinates });
                    
                }
                else {
                    bus_commands.push_back(command);
                }
            }

            FillDistances(catalogue);

            for (const CommandDescription& command : bus_commands) {
                std::vector<std::string_view> temp_stops = ParseRoute(command.description);
                std::vector<const Stop*> bus_stops;

                for (std::string_view stop : temp_stops) {
                    bus_stops.push_back(catalogue.GetStop(stop));
                }

                catalogue.AddBus({ command.id, bus_stops });
            }
        }

        std::vector<CommandDescription> InputReader::GetCommands() const {
            return commands_;
        }

        void FillCatalogue(std::istream& input, TransportCatalogue& catalogue) {
            int base_request_count;
            input >> base_request_count >> std::ws;

            InputReader reader;
            for (int i = 0; i < base_request_count; ++i) {
                std::string line;
                getline(input, line);
                reader.ParseLine(line);
            }
            reader.ApplyCommands(catalogue);
        }
    }
}
