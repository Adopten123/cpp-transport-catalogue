// Вставьте сюда решение из предыдущего спринта

#include "transport_router.h"

namespace transport {

	void TransportRouter::FillGraphByStops(const std::map<std::string_view, domain::Stop*>& stops,
		TransportRouter::Graph& graph) {
		using namespace std;
		using namespace graph;

		map<string, VertexId> stop_ids;
		VertexId vertex_id = 0;

		for (const auto& [name, info] : stops) {
			stop_ids[info->name_] = vertex_id;

			graph.AddEdge({
				.name = info->name_,
				.quality = 0,
				.from = vertex_id,
				.to = ++vertex_id,
				.weight = static_cast<double>(settings_.bus_wait_time),
			});

			++vertex_id;
		}
		stop_ids_ = move(stop_ids);
	}

	void TransportRouter::FillGraphByBus(const std::map<std::string_view, domain::Bus*>& buses,
		TransportRouter::Graph& graph, const TransportCatalogue& catalogue) {
		using namespace std;
		using namespace graph;

		for (const auto& [name, bus] : buses) {

			const vector<const domain::Stop*>& stops = bus->stops_;
			size_t stops_count = bus->stops_.size();

			for (size_t i_from = 0; i_from < stops_count; i_from++) {

				for (size_t i_to = i_from + 1; i_to < stops_count; i_to++) {

					size_t road_distance = 0;
					size_t road_distance_inverse = 0;

					for (size_t i = i_from + 1; i <= i_to; i++) {

						road_distance += catalogue.GetDistance(
							const_cast<domain::Stop*>(stops[i - 1]),
							const_cast<domain::Stop*>(stops[i])
						);
						road_distance_inverse += catalogue.GetDistance(
							const_cast<domain::Stop*>(stops[i]),
							const_cast<domain::Stop*>(stops[i - 1])
						);

					}

					graph.AddEdge({
						.name = bus->name_,
						.quality = i_to - i_from,
						.from = stop_ids_.at(stops[i_from]->name_) + 1,
						.to = stop_ids_.at(stops[i_to]->name_),
						.weight = static_cast<double>(road_distance) / (settings_.bus_velocity * (100.0 / 6.0))
						});

					if (!bus->is_circular_) {
						graph.AddEdge({
						.name = bus->name_,
						.quality = i_to - i_from,
						.from = stop_ids_.at(stops[i_to]->name_) + 1,
						.to = stop_ids_.at(stops[i_from]->name_),
						.weight = static_cast<double>(road_distance_inverse) / (settings_.bus_velocity * (100.0 / 6.0))
						});
					}

				}
			}
		}
	}

	void TransportRouter::BuildGraph(const TransportCatalogue& catalogue) {
		using namespace std;
		using namespace graph;

		const auto& buses = catalogue.GetAllBuses();
		const auto& stops = catalogue.GetAllStops();
		
		Graph graph(stops.size() * 2);

		FillGraphByStops(stops, graph);

		FillGraphByBus(buses, graph, catalogue);

		graph_ = std::move(graph);
	}

	const TransportRouter::Graph& TransportRouter::GetGraph() const {
		return graph_;
	}

	const std::optional<TransportRouter::Router::RouteInfo> TransportRouter::FindBus(const std::string& from, const std::string& to) const {
		return router_->BuildRoute(stop_ids_.at(from), stop_ids_.at(to));
	}



}