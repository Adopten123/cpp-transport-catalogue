#include "request_handler.h"

namespace transport {

	RequestHandler::RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer, const TransportRouter& tr) :
		db_(db), renderer_(renderer), tr_(tr) {
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

	const std::optional<RequestHandler::Router::RouteInfo> RequestHandler::GetRoute(const std::string& from, const std::string& to) const {
		return tr_.FindBus(from, to);
	}

	const RequestHandler::Graph& RequestHandler::GetGraph() const {
		return tr_.GetGraph();
	}

}