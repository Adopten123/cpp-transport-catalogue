#include "domain.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области
 * (domain) вашего приложения и не зависят от транспортного справочника. Например Автобусные
 * маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

namespace domain {

	Stop::Stop(const std::string& name, const transport::geo::Coordinates& coordinate) :
		name_(name), coordinate_(coordinate) {}

	Bus::Bus(const std::string& name, const std::vector<const Stop*>& stops) :
		name_(name), stops_(stops) {
	}

	bool Bus::operator<(Bus& other) {
		return std::lexicographical_compare(name_.begin(), name_.end(),
			other.name_.begin(), other.name_.end());
	}

	BusInfo::BusInfo(size_t stops_count, size_t unique_stops_count, double route_length, double curvature) :
		stops_count_(stops_count), unique_stops_count_(unique_stops_count),
		route_length_(route_length), curvature_(curvature) {}

}