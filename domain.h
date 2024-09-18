#pragma once

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

#include "geo.h"

#include <set>
#include <string>
#include <vector>


namespace domain {

	struct Stop
	{
		Stop() = default;

		Stop(const std::string& name, const transport::geo::Coordinates& coordinate);

		std::string name_;
		transport::geo::Coordinates coordinate_;
		std::set<std::string_view> buses_by_stop_;
	};

	struct Bus
	{
		Bus() = default;

		Bus(const std::string& name, const std::vector<const Stop*>& stops);

		bool operator<(Bus& other);

		bool is_circular_;
		std::string name_;
		std::vector<const Stop*> stops_;
	};

	struct BusInfo {
		BusInfo() = default;

		BusInfo(size_t stops_count, size_t unique_stops_count, double route_length, double curvature);

		size_t stops_count_ = 0u;
		size_t unique_stops_count_ = 0u;
		double geo_route_length_ = 0.0;
		double route_length_ = 0.0;
		double curvature_ = 0.0;
	};

}