#include "transport_catalogue.h"

#include <algorithm>
#include <memory>
#include <set>
#include <stdexcept>


namespace transport {

	void TransportCatalogue::AddStop(domain::Stop&& stop) {
		stops_.push_back(std::move(stop));
		stopname_to_stop_[stops_.back().name_] = &stops_.back();

		stop_to_buses_[stops_.back().name_] = std::set<domain::Bus*, BusPtrHasher>();

	}

	void TransportCatalogue::AddBus(domain::Bus&& bus) {

		auto& bus_ref = buses_.emplace_back(std::move(bus));
		busname_to_bus_[buses_.back().name_] = &buses_.back();

		for (const domain::Stop* stop : buses_.back().stops_) {
			stop_to_buses_[stop->name_].insert(&buses_.back());
		}

		if (!bus_ref.is_circular_)
		{
			for (int i = bus_ref.stops_.size() - 2; i >= 0; --i)
			{
				bus_ref.stops_.push_back(bus_ref.stops_[i]);
			}
		}

	}

	void TransportCatalogue::SetDistance(domain::Stop* from, domain::Stop* to, size_t distance) {
		distances_[std::make_pair(from, to)] = distance;
	}

	const domain::Stop* TransportCatalogue::GetStop(std::string_view name) const {
		auto pair = stopname_to_stop_.find(name);
		return pair != stopname_to_stop_.end() ? pair->second : nullptr;
	}

	const domain::Bus* TransportCatalogue::GetBus(std::string_view name) const {
		auto pair = busname_to_bus_.find(name);
		return pair != busname_to_bus_.end() ? pair->second : nullptr;
	}

	const std::set<domain::Bus*, BusPtrHasher>& TransportCatalogue::GetBusesByStop(std::string_view name) const {
		auto iterator = stop_to_buses_.find(name);
		if (iterator != stop_to_buses_.end()) {
			return iterator->second;
		}
		else {
			static const std::set<domain::Bus*, BusPtrHasher> empty_set;
			return empty_set;
		}
	}

	size_t TransportCatalogue::GetDistanceDirectly(domain::Stop* from, domain::Stop* to) const{
		if (distances_.count(std::make_pair(from, to)) > 0) {
			return distances_.at(std::make_pair(from, to));
		}
		else {
			return 0U;
		}
	}

	size_t TransportCatalogue::GetDistance(domain::Stop* from, domain::Stop* to) const {
		size_t result = GetDistanceDirectly(from, to);
		return (result > 0 ? result : GetDistanceDirectly(to, from));
	}

	domain::BusInfo TransportCatalogue::GetBusInfo(const domain::Bus* bus) const {

		domain::BusInfo bus_info;

		bus_info.stops_count_ = bus->stops_.size();

		std::set<const domain::Stop*> uniq_stops(bus->stops_.begin(), bus->stops_.end());
		bus_info.unique_stops_count_ = uniq_stops.size();

		for (size_t i = 1; i < bus->stops_.size(); ++i) {
			domain::Stop* prev_stop = const_cast<domain::Stop*>(bus->stops_[i - 1]);
			domain::Stop* curr_stop = const_cast<domain::Stop*>(bus->stops_[i]);

			bus_info.route_length_ += GetDistance(prev_stop, curr_stop);
			bus_info.geo_route_length_ += transport::geo::ComputeDistance(prev_stop->coordinate_, curr_stop->coordinate_);
		}

		bus_info.curvature_ = bus_info.route_length_ / bus_info.geo_route_length_;

		return bus_info;
	}

	const std::map<std::string_view, domain::Bus*> TransportCatalogue::GetAllBuses() const {
		std::map<std::string_view, domain::Bus*> result;

		for (const auto& bus : busname_to_bus_) {
			result.emplace(bus);
		}

		return result;
	}

}
