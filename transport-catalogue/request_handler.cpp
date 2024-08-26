#include "request_handler.h"

namespace transport {

	RequestHandler::RequestHandler(const TransportCatalogue& db) :
		db_(db) {
	}

	std::optional<domain::BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
		return db_.GetBusInfo(db_.GetBus(bus_name));
	}

	const std::set<domain::Bus*, BusPtrHasher>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
		return &(db_.GetBusesByStop(stop_name));
	}

}