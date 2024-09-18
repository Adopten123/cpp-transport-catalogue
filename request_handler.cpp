#include "request_handler.h"

namespace transport {

	RequestHandler::RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer) :
		db_(db), renderer_(renderer) {
	}

	std::optional<domain::BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
		auto bus = db_.GetBus(bus_name);
		if (!bus) {
			return std::nullopt;
		}

		auto bus_stat = db_.GetBusInfo(bus);
		return bus_stat;
	}

	std::optional <const std::set<domain::Bus*, BusPtrHasher>*> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
		if (!db_.GetStop(stop_name)) {
			return std::nullopt;
		}

		auto buses_by_stop = &db_.GetBusesByStop(stop_name);
		return buses_by_stop;
	}

	svg::Document RequestHandler::RenderMap() const {
		return renderer_.RenderSVG(db_.GetAllBuses());
	}

}