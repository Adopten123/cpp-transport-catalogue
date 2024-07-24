#pragma once

#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "geo.h"
#include "transport_catalogue.h"

namespace transport {
    namespace reader {
        struct CommandDescription {
            explicit operator bool() const {
                return !command.empty();
            }

            bool operator!() const {
                return !operator bool();
            }

            std::string command;
            std::string id;
            std::string description;
        };

        class InputReader {
        public:
            void ParseLine(std::string_view line);

            void FillDistances(TransportCatalogue& catalogue) const;
            void ApplyCommands(TransportCatalogue& catalogue) const;

            std::vector<CommandDescription> GetCommands() const;

        private:
            std::vector<CommandDescription> commands_;
        };

        void FillCatalogue(std::istream& input, TransportCatalogue& catalogue);

        std::unordered_map<std::string, size_t> ParseDistances(std::string_view str);
    }
}
