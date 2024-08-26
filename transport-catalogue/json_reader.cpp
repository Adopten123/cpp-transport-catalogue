#include "json_reader.h"

#include <set>

namespace transport {
	namespace reader {

		using namespace std::literals;

		void JSONreader::ProcessJSON(TransportCatalogue& tc, RequestHandler& rh,
			std::istream& input, std::ostream& output) {

			const json::Dict json_dict = json::Load(input).GetRoot().AsMap();

			const auto base_requests_it = json_dict.find("base_requests"s);

			if (base_requests_it != json_dict.end()) {
				FillCatalogue(tc,base_requests_it->second.AsArray());
			}

			const auto stat_requests_it = json_dict.find("stat_requests"s);

			if (stat_requests_it != json_dict.end()) {
				ProcessQueries(output, rh, stat_requests_it->second.AsArray());
			}

		}

		void JSONreader::FillCatalogue(TransportCatalogue& tc, const json::Array& json_arr) {

			json::Array bus_arr;

			for (const auto& item : json_arr) {

				const auto request_type = item.AsMap().find("type"s);

				if (request_type != item.AsMap().end() and request_type->second.AsString() == "Stop"s) {
					AddStopData(tc, item.AsMap());
				}

				if (request_type->second.AsString() == "Bus"s) {
					bus_arr.push_back(item);
				}
			}

			for (const auto& item : json_arr) {

				const auto request_type = item.AsMap().find("type"s);

				if (request_type != item.AsMap().end() and request_type->second.AsString() == "Stop"s) {
					AddStopDistance(tc, item.AsMap());
				}

			}

			for (const auto& item : bus_arr) {

				const auto request_type = item.AsMap().find("type"s);

				if (request_type != item.AsMap().end() and request_type->second.AsString() == "Bus"s) {
					AddBusData(tc, item.AsMap());
				}

			}

		}

		void JSONreader::AddStopData(TransportCatalogue& tc, const json::Dict& json_stop) {
			tc.AddStop(domain::Stop{ json_stop.at("name"s).AsString(),
				{json_stop.at("latitude"s).AsDouble(),
				json_stop.at("longitude"s).AsDouble()} });
		}

		void JSONreader::AddStopDistance(TransportCatalogue& tc, const json::Dict& json_stop) {

			const domain::Stop* from_ptr = tc.GetStop(json_stop.at("name"s).AsString());

			if (from_ptr) {
				const json::Dict stops = json_stop.at("road_distances"s).AsMap();
				for (const auto& [stop, distance] : stops) {
					tc.SetDistance(const_cast<domain::Stop*>(from_ptr),
						const_cast<domain::Stop*>(tc.GetStop(stop)), static_cast<size_t>(distance.AsInt()));
				}
			}
		}

		void JSONreader::AddBusData(TransportCatalogue& tc, const json::Dict& json_bus) {
			domain::Bus bus;
			bus.name_ = json_bus.at("name"s).AsString();
			bus.is_circular_ = json_bus.at("is_roundtrip"s).AsBool();

			for (auto& item : json_bus.at("stops"s).AsArray()) {
				domain::Stop* tmp_ptr = const_cast<domain::Stop*>(tc.GetStop(item.AsString()));
				if (tmp_ptr) {
					bus.stops_.push_back(tmp_ptr);
				}
			}
			tc.AddBus(std::move(bus));
		}

		void JSONreader::ProcessQueries(std::ostream& out, RequestHandler& rh, const json::Array& json_arr) {
			
			json::Array completed_queries;

			for (const auto& query : json_arr) {
				const auto request_type = query.AsMap().find("type"s);

				if (request_type != query.AsMap().cend()) {
					if (request_type->second.AsString() == "Stop"s) {
						completed_queries.emplace_back(ProcessStopQuery(rh,query.AsMap()));
					}
					else if (request_type->second.AsString() == "Bus"s) {
						completed_queries.emplace_back(ProcessBusQuery(rh,query.AsMap()));
					}
				}
			}

			json::Print(json::Document{ completed_queries }, out);
		}

		const json::Node JSONreader::ProcessStopQuery(RequestHandler& rh, const json::Dict& json_stop) {
			const auto stop_query_ptr = rh.GetBusesByStop(
				json_stop.at("name"s).AsString()
			);

			if (stop_query_ptr == nullptr) {
				return json::Dict{ { "request_id"s, json_stop.at("id"s).AsInt() },
					{"error_message"s, "not found"s} };
			}

			json::Array routes;
			for (const auto& bus : *stop_query_ptr)
			{
				routes.push_back(bus->name_);
			}
			return json::Dict{ {"buses"s, routes},
							  {"request_id"s, json_stop.at("id"s).AsInt()} };

		}

		const json::Node JSONreader::ProcessBusQuery(RequestHandler& rh, const json::Dict& json_bus) {
			const auto bus_query_ptr = rh.GetBusStat(
				json_bus.at("name"s).AsString()
			);

			if (bus_query_ptr) {
				return json::Dict{ {"curvature"s, bus_query_ptr->curvature_},
					  {"request_id"s, json_bus.at("id"s).AsInt()},
					  {"route_length"s, static_cast<int>(bus_query_ptr->geo_route_length_)},
					  {"stop_count"s, static_cast<int>(bus_query_ptr->stops_count_)},
					  {"unique_stop_count"s, static_cast<int>(bus_query_ptr->unique_stops_count_)} };
			}

			return json::Dict{ {"request_id"s, json_bus.at("id"s).AsInt()},
						  {"error_message"s, "not found"s} };
		}
	}
}