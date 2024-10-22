#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <map>
#include <memory>
#include <string>


namespace transport 
{
	struct RouterSettings {
		int bus_wait_time = 0;
		double bus_velocity = 0.0;
	};

	class TransportRouter {
	private:
		using Graph = graph::DirectedWeightedGraph<double>;
		using Router = graph::Router<double>;

	public:

		TransportRouter() = default;

		TransportRouter(RouterSettings settings)
			: settings_(settings) {
		}

		TransportRouter(const TransportRouter& router, const TransportCatalogue& catalogue)
			: settings_(router.settings_){

			BuildGraph(catalogue);
			router_ = std::make_unique<Router>(graph_);
		}
		
		const Graph& GetGraph() const;

		const std::optional<Router::RouteInfo> FindBus(const std::string& from, const std::string& to) const;

	private:
		RouterSettings settings_;

		Graph graph_;
		std::map<std::string, graph::VertexId> stop_ids_;
		std::unique_ptr<Router> router_;

		void FillGraphByStops(const std::map<std::string_view, domain::Stop*>& stops,
			Graph& graph);

		void FillGraphByBus(const std::map<std::string_view, domain::Bus*>& buses,
			Graph& graph, const TransportCatalogue& catalogue);

		void BuildGraph(const TransportCatalogue& catalogue);
	};
}