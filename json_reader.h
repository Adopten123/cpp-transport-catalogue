#pragma once

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

#include <sstream>

namespace transport {
    namespace reader {
        class JSONreader {
        public:

            void ProcessJSON(TransportCatalogue& tc, RequestHandler& rh, renderer::MapRenderer& mr,
                std::istream& input, std::ostream& output);

        private:

            void AddStopData(TransportCatalogue& tc, const json::Dict& json_stop);

            void AddStopDistance(TransportCatalogue& tc, const json::Dict& json_stop);

            void AddBusData(TransportCatalogue& tc, const json::Dict& json_bus);

            void FillCatalogue(TransportCatalogue& tc, const json::Array& json_arr);

            const json::Node ProcessStopQuery(RequestHandler& rh, const json::Dict& json_stop);
            const json::Node ProcessBusQuery(RequestHandler& rh, const json::Dict& json_bus);
            const json::Node ProcessMapQuery(RequestHandler& rh, const json::Dict& json_map);

            void ProcessQueries(std::ostream& out, RequestHandler& rh, const json::Array& json_arr);

            const svg::Color GetColor(const json::Node& color);

            void LoadRendererSettings(renderer::MapRenderer& mr, const json::Dict& json_dict);


        };
    }
    

}  // namespace transport
