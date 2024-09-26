#include "json_reader.h"

#include <set>
#include "json_builder.h"

namespace transport {
	namespace reader {

		using namespace std::literals;

		void JsonReader::ProcessJSON(TransportCatalogue& tc, RequestHandler& rh, renderer::MapRenderer & mr,
			std::istream& input, std::ostream& output) {

			const json::Dict json_dict = json::Load(input).GetRoot().AsDict();

			const auto base_requests_it = json_dict.find("base_requests"s);

			if (base_requests_it != json_dict.end()) {
				FillCatalogue(tc,base_requests_it->second.AsArray());
			}

			const auto renderer_settings_it = json_dict.find("render_settings"s);
			if (renderer_settings_it != json_dict.cend())
			{
				LoadRendererSettings(mr, renderer_settings_it->second.AsDict());
			}

			const auto stat_requests_it = json_dict.find("stat_requests"s);

			if (stat_requests_it != json_dict.end()) {
				ProcessQueries(output, rh, stat_requests_it->second.AsArray());
			}

		}

		void JsonReader::FillCatalogue(TransportCatalogue& tc, const json::Array& json_arr) {

			json::Array bus_arr;

			for (const auto& item : json_arr) {

				const auto request_type = item.AsDict().find("type"s);

				if (request_type != item.AsDict().end() and request_type->second.AsString() == "Stop"s) {
					AddStopData(tc, item.AsDict());
				}

				if (request_type->second.AsString() == "Bus"s) {
					bus_arr.push_back(item);
				}
			}

			for (const auto& item : json_arr) {

				const auto request_type = item.AsDict().find("type"s);

				if (request_type != item.AsDict().end() and request_type->second.AsString() == "Stop"s) {
					AddStopDistance(tc, item.AsDict());
				}

			}

			for (const auto& item : bus_arr) {

				const auto request_type = item.AsDict().find("type"s);

				if (request_type != item.AsDict().end()) {
					AddBusData(tc, item.AsDict());
				}

			}

		}

		void JsonReader::AddStopData(TransportCatalogue& tc, const json::Dict& json_stop) {
			tc.AddStop(domain::Stop{ json_stop.at("name"s).AsString(),
				{json_stop.at("latitude"s).AsDouble(),
				json_stop.at("longitude"s).AsDouble()} });
		}

		void JsonReader::AddStopDistance(TransportCatalogue& tc, const json::Dict& json_stop) {

			const domain::Stop* from_ptr = tc.GetStop(json_stop.at("name"s).AsString());

			if (from_ptr) {
				const json::Dict stops = json_stop.at("road_distances"s).AsDict();
				for (const auto& [stop, distance] : stops) {
					tc.SetDistance(const_cast<domain::Stop*>(from_ptr),
						const_cast<domain::Stop*>(tc.GetStop(stop)), static_cast<size_t>(distance.AsInt()));
				}
			}
		}

		void JsonReader::AddBusData(TransportCatalogue& tc, const json::Dict& json_bus) {
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

		void JsonReader::ProcessQueries(std::ostream& out, RequestHandler& rh, const json::Array& json_arr) {

			json::Array completed_queries;
			//int c = 0;
			try {
				for (const auto& query : json_arr) {
					const auto request_type = query.AsDict().find("type"s);
					if (request_type != query.AsDict().cend()) {

						if (request_type->second.AsString() == "Stop"s) {
							completed_queries.emplace_back(ProcessStopQuery(rh, query.AsDict()));
						}

						else if (request_type->second.AsString() == "Bus"s) {
							completed_queries.emplace_back(ProcessBusQuery(rh, query.AsDict()));
						}

						else if (request_type->second.AsString() == "Map"s) {
							completed_queries.emplace_back(ProcessMapQuery(rh, query.AsDict()));
						}

					}
					//c++;
				}
				json::Print(json::Document{ completed_queries }, out);
			}
			catch(...){
				//std::cerr << c << std::endl;
			}


		}

		const json::Node JsonReader::ProcessStopQuery(RequestHandler& rh, const json::Dict& json_stop) {
			
			const auto stop_query_ptr = rh.GetBusesByStop(
				json_stop.at("name"s).AsString()
			);

			if (stop_query_ptr == std::nullopt)
			{
				return json::Node{ json::Builder{}
								.StartDict()
									.Key("error_message"s).Value("not found"s)
									.Key("request_id"s).Value(json_stop.at("id").AsInt())
								.EndDict()
					            .Build()
				};
			}
			else {
				json::Array routes;
				for (const auto& bus : *(stop_query_ptr.value()))
				{
					routes.push_back(bus->name_);
				}

				return json::Node{ json::Builder{}
									.StartDict()
										.Key("buses"s).Value(routes)
										.Key("request_id"s).Value(json_stop.at("id").AsInt())
									.EndDict()
					                .Build()
				};
			}

		}

		const json::Node JsonReader::ProcessBusQuery(RequestHandler& rh, const json::Dict& json_bus) {

			if (const auto bus_query_ptr = rh.GetBusStat(
				json_bus.at("name"s).AsString())) {

				return json::Node{ json::Builder{}
								.StartDict()
									.Key("curvature"s).Value(bus_query_ptr.value().curvature_)
									.Key("request_id"s).Value(json_bus.at("id").AsInt())
									.Key("route_length"s).Value(static_cast<int>(bus_query_ptr.value().route_length_))
									.Key("stop_count"s).Value(static_cast<int>(bus_query_ptr.value().stops_count_))
									.Key("unique_stop_count"s).Value(static_cast<int>(bus_query_ptr.value().unique_stops_count_))
								.EndDict()
					            .Build()
				};

			}

			return json::Node{ json::Builder{}
								.StartDict()
									.Key("error_message"s).Value("not found"s)
									.Key("request_id"s).Value(json_bus.at("id").AsInt())
								.EndDict()
				                .Build()
			};
		}

		const json::Node JsonReader::ProcessMapQuery(RequestHandler& rh, const json::Dict& json_map) {

			svg::Document svg_map = rh.RenderMap();
			std::ostringstream os_stream;
			svg_map.Render(os_stream);

			return json::Node{ json::Builder{}
											.StartDict()
												.Key("map"s).Value(os_stream.str())
												.Key("request_id"s).Value(json_map.at("id"s).AsInt())
											.EndDict()
				                            .Build() };
		}

		const svg::Color JsonReader::GetColor(const json::Node& color)
		{
			if (color.IsString())
			{
				return svg::Color{ color.AsString() };
			}
			else if (color.IsArray())
			{
				if (color.AsArray().size() == 3)
				{
					return svg::Rgb
					{
						static_cast<uint8_t>(color.AsArray()[0].AsInt()),
						static_cast<uint8_t>(color.AsArray()[1].AsInt()),
						static_cast<uint8_t>(color.AsArray()[2].AsInt())
					};
				}
				else if (color.AsArray().size() == 4)
				{
					return svg::Rgba
					{
						static_cast<uint8_t>(color.AsArray()[0].AsInt()),
						static_cast<uint8_t>(color.AsArray()[1].AsInt()),
						static_cast<uint8_t>(color.AsArray()[2].AsInt()),
						color.AsArray()[3].AsDouble()
					};
				}
			}
			return svg::Color();
		}

		void JsonReader::LoadRendererSettings(renderer::MapRenderer& mr, const json::Dict& json_dict) {
			renderer::RendererSettings loaded_settings;

			loaded_settings.width = json_dict.at("width").AsDouble();
			loaded_settings.height = json_dict.at("height").AsDouble();
			loaded_settings.padding = json_dict.at("padding").AsDouble();
			loaded_settings.line_width = json_dict.at("line_width").AsDouble();
			loaded_settings.stop_radius = json_dict.at("stop_radius").AsDouble();
			loaded_settings.bus_label_font_size = json_dict.at("bus_label_font_size").AsInt();
			loaded_settings.bus_label_offset = { json_dict.at("bus_label_offset").AsArray()[0].AsDouble(), json_dict.at("bus_label_offset").AsArray()[1].AsDouble() };
			loaded_settings.stop_label_font_size = json_dict.at("stop_label_font_size").AsInt();
			loaded_settings.stop_label_offset = { json_dict.at("stop_label_offset").AsArray()[0].AsDouble(), json_dict.at("stop_label_offset").AsArray()[1].AsDouble() };
			loaded_settings.underlayer_color = GetColor(json_dict.at("underlayer_color"));
			loaded_settings.underlayer_width = json_dict.at("underlayer_width").AsDouble();
			loaded_settings.color_palette.clear();

			for (const auto& color : json_dict.at("color_palette").AsArray())
			{
				loaded_settings.color_palette.emplace_back(GetColor(color));
			}

			mr = loaded_settings;
		}
	}
}