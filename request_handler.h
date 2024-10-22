#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <map>
#include <optional>
#include <set>

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)


namespace transport {

    class RequestHandler {
    private:
        using Graph = graph::DirectedWeightedGraph<double>;
        using Router = graph::Router<double>;

    public:

        RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer, const TransportRouter& tr);

        // Возвращает информацию о маршруте (запрос Bus)
        std::optional<domain::BusInfo> GetBusStat(const std::string_view& bus_name) const;

        // Возвращает маршруты, проходящие через
        std::optional < const std::set<domain::Bus*, BusPtrHasher>*> GetBusesByStop(const std::string_view& stop_name) const;

        // Этот метод будет нужен в следующей части итогового проекта
        svg::Document RenderMap() const;

        const std::optional<Router::RouteInfo> GetRoute(const std::string& from, const std::string& to) const;
        const Graph& GetGraph() const;

    private:
        // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
        const TransportCatalogue& db_;
        const renderer::MapRenderer& renderer_;
        const TransportRouter& tr_;
    };

}

