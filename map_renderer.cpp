#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace transport {
	namespace renderer {
		using namespace std::literals;
		//------------------------ IsZero -------------------------

		bool IsZero(double value) {
			return std::abs(value) < EPSILON;
		}

		//------------------------ SphereProjector -------------------------

		svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
			return {
				(coords.lng - min_lon_) * zoom_coeff_ + padding_,
				(max_lat_ - coords.lat) * zoom_coeff_ + padding_
			};
		}

		//------------------------ RoteLine -------------------------

		RouteLine::RouteLine(const std::vector<svg::Point> points, svg::Color stroke_color, const RendererSettings& settings) :
			points_(points), stroke_color_(stroke_color), settings_(settings){}

		void RouteLine::Draw(svg::ObjectContainer& container) const
		{
			svg::Polyline polyline;
			for (const auto& point : points_)
			{
				polyline.AddPoint(point);
			}
			polyline.SetStrokeColor(stroke_color_);
			polyline.SetFillColor(svg::NoneColor);                  
			polyline.SetStrokeWidth(settings_.line_width);
			polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			container.Add(polyline);
		}

		//------------------------ MapRenderer -------------------------
		MapRenderer::MapRenderer(const RendererSettings& settings) :
			settings_(settings){}


		svg::Polyline MapRenderer::RenderPolyline(std::vector<const domain::Stop*> route_stops, size_t& color_count, const SphereProjector& sp) const {
			svg::Polyline temp_line;

			for (const auto& stop : route_stops) {
				temp_line.AddPoint(sp(stop->coordinate_));
			}

			temp_line.SetStrokeColor(settings_.color_palette[color_count]);
			temp_line.SetFillColor("none");
			temp_line.SetStrokeWidth(settings_.line_width);
			temp_line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			temp_line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			if (color_count < (settings_.color_palette.size() - 1)) {
				color_count++;
			} else {
				color_count = 0;
			}

			return temp_line;
		}

		std::vector<svg::Polyline> MapRenderer::GetRouteLines(
			const std::map<std::string_view, domain::Bus*>& buses, const SphereProjector& sp) const {

			std::vector<svg::Polyline> result;
			size_t color_count = 0;

			for (const auto& [busname, bus] : buses) {

				if (bus->stops_.empty()) {
					continue;
				}

				std::vector<const domain::Stop*> route_stops { bus->stops_.begin(), bus->stops_.end() };

				svg::Polyline line = RenderPolyline(route_stops, color_count, sp);
				
				result.push_back(line);
			}

			return result;
		}

		svg::Text MapRenderer::FillBusnameText(const domain::Bus* bus, const size_t& color_count, const SphereProjector& sp) const {
			svg::Text text;

			text.SetPosition(sp(bus->stops_[0]->coordinate_));
			text.SetOffset(settings_.bus_label_offset);
			text.SetFontSize(settings_.bus_label_font_size);
			text.SetFontFamily("Verdana");
			text.SetFontWeight("bold");
			text.SetData(bus->name_);
			text.SetFillColor(settings_.color_palette[color_count]);

			return text;
		}

		svg::Text MapRenderer::FillBusnameUnderlabel(const domain::Bus* bus, const SphereProjector& sp) const {

			svg::Text underlabel;

			underlabel.SetPosition(sp(bus->stops_[0]->coordinate_));
			underlabel.SetOffset(settings_.bus_label_offset);
			underlabel.SetFontSize(settings_.bus_label_font_size);
			underlabel.SetFontFamily("Verdana");
			underlabel.SetFontWeight("bold");
			underlabel.SetData(bus->name_);
			underlabel.SetFillColor(settings_.underlayer_color);
			underlabel.SetStrokeColor(settings_.underlayer_color);
			underlabel.SetStrokeWidth(settings_.underlayer_width);
			underlabel.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			underlabel.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			return underlabel;
		}

		std::vector<svg::Text> MapRenderer::GetBusnameLabels(const std::map<std::string_view, domain::Bus*>& buses, const SphereProjector& sp) const {
			std::vector<svg::Text> result;
			size_t color_count = 0;

			for (const auto& [busname, bus] : buses) {
				if (bus->stops_.empty()) {
					continue;
				}

				svg::Text text = FillBusnameText(bus, color_count, sp);


				if (color_count < (settings_.color_palette.size() - 1)) {
					color_count++;
				} 
				else {
					color_count = 0;
				}

				svg::Text underlabel = FillBusnameUnderlabel(bus, sp);

				result.push_back(underlabel);
				result.push_back(text);

				if (bus->is_circular_ == false
					and bus->stops_[0] != bus->stops_[bus->stops_.size() / 2]) {

					svg::Text clone_text{ text };
					svg::Text clone_underlabel{ underlabel };

					clone_text.SetPosition(sp(bus->stops_[bus->stops_.size() / 2]->coordinate_));
					clone_underlabel.SetPosition(sp(bus->stops_[bus->stops_.size() / 2]->coordinate_));

					result.push_back(clone_underlabel);
					result.push_back(clone_text);
				}

			}

			return result;
		}

		std::vector<svg::Circle> MapRenderer::GetStopIcons(const std::map<std::string_view, const domain::Stop*>& stops, const SphereProjector& sp) const {
			std::vector<svg::Circle> result;

			for (const auto& [stop_name, stop] : stops) {
				svg::Circle icon;

				icon.SetCenter(sp(stop->coordinate_));
				icon.SetRadius(settings_.stop_radius);
				icon.SetFillColor("white");

				result.push_back(icon);
			}

			return result;
		}

		svg::Text MapRenderer::FillStopnameText(const domain::Stop* stop, const SphereProjector& sp) const {
			svg::Text text;
			
			text.SetPosition(sp(stop->coordinate_));
			text.SetOffset(settings_.stop_label_offset);
			text.SetFontSize(settings_.stop_label_font_size);
			text.SetFontFamily("Verdana");
			text.SetData(stop->name_);
			text.SetFillColor("black");

			return text;
		}
		svg::Text MapRenderer::FillStopnameUnderlabel(const domain::Stop* stop, const SphereProjector& sp) const {

			svg::Text underlabel;

			underlabel.SetPosition(sp(stop->coordinate_));
			underlabel.SetOffset(settings_.stop_label_offset);
			underlabel.SetFontSize(settings_.stop_label_font_size);
			underlabel.SetFontFamily("Verdana");
			underlabel.SetData(stop->name_);
			underlabel.SetFillColor(settings_.underlayer_color);
			underlabel.SetStrokeColor(settings_.underlayer_color);
			underlabel.SetStrokeWidth(settings_.underlayer_width);
			underlabel.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			underlabel.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			return underlabel;
		}

		std::vector<svg::Text> MapRenderer::GetStopnameLabels(const std::map<std::string_view, const domain::Stop*>& stops, const SphereProjector& sp) const {
			std::vector<svg::Text> result;

			for (const auto& [stop_name, stop] : stops) {
				svg::Text text = FillStopnameText(stop, sp);

				svg::Text underlabel = FillStopnameUnderlabel(stop, sp);

				result.push_back(underlabel);
				result.push_back(text);
			}

			return result;
		}

		svg::Document MapRenderer::RenderSVG(const std::map<std::string_view, domain::Bus*> busname_to_bus) const {
			svg::Document result_doc;
			std::unordered_set<geo::Coordinates, geo::CoordinateHasher> stops_coords;
			std::map<std::string_view, const domain::Stop*> all_stops;

			for(const auto& [busname, bus] : busname_to_bus){

				if (bus->stops_.empty()) {
					continue;
				}

				for (const auto& stop : bus->stops_) {
					stops_coords.insert(stop->coordinate_);
					all_stops[stop->name_] = stop;
				}
			}

			SphereProjector sp(std::begin(stops_coords), std::end(stops_coords),
				settings_.width, settings_.height, settings_.padding);

			for (const auto& line : GetRouteLines(busname_to_bus, sp)) {
				result_doc.Add(line);
			}

			for (const auto& text : GetBusnameLabels(busname_to_bus, sp)) {
				result_doc.Add(text);
			}

			for (const auto& circle : GetStopIcons(all_stops, sp)) {
				result_doc.Add(circle);
			}
			for (const auto& text : GetStopnameLabels(all_stops, sp)) {
				result_doc.Add(text);
			}

			return result_doc;
		}

	} // renderer namespace
} // transport namespace