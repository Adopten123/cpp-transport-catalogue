#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"

namespace transport {
    namespace reader {
        class JSONreader {
        public:

            void ProcessJSON(TransportCatalogue& tc, RequestHandler& rh,
                std::istream& input, std::ostream& output);

        private:

            void AddStopData(TransportCatalogue& tc, const json::Dict& json_stop);

            void AddStopDistance(TransportCatalogue& tc, const json::Dict& json_stop);

            void AddBusData(TransportCatalogue& tc, const json::Dict& json_bus);

            void FillCatalogue(TransportCatalogue& tc, const json::Array& json_arr);

            const json::Node ProcessStopQuery(RequestHandler& rh, const json::Dict& json_stop);
            const json::Node ProcessBusQuery(RequestHandler& rh, const json::Dict& json_bus);

            void ProcessQueries(std::ostream& out, RequestHandler& rh, const json::Array& json_arr);


        };
    }
    

}  // namespace transport
