#include "transport_catalogue.h"

#include <algorithm>
#include <set>


namespace transport {

	void TransportCatalogue::AddStop(Stop&& stop) {
		stops_.push_back(std::move(stop));
		stopname_to_stop_[stops_.back().name_] = &stops_.back();

		stop_to_buses_[stops_.back().name_] = std::set<Bus*, BusPtrHasher>();

	}

	void TransportCatalogue::AddBus(Bus&& bus) {
		buses_.push_back(std::move(bus));
		busname_to_bus_[buses_.back().name_] = &buses_.back();

		for (const Stop* stop : buses_.back().stops_) {
			stop_to_buses_[stop->name_].insert(&buses_.back());
		}
	}

	void TransportCatalogue::SetDistance(Stop* from, Stop* to, size_t distance) {
		distances_[std::make_pair(from, to)] = distance;
	}

	const Stop* TransportCatalogue::GetStop(std::string_view name) const {
		return stopname_to_stop_.count(name) ? stopname_to_stop_.at(name) : nullptr;
	}
	const Bus* TransportCatalogue::GetBus(std::string_view name) const {
		return busname_to_bus_.count(name) ? busname_to_bus_.at(name) : nullptr;
	}

	const std::set<Bus*, BusPtrHasher>& TransportCatalogue::GetBusesByStop(std::string_view name) const {
		auto iterator = stop_to_buses_.find(name);
		if (iterator != stop_to_buses_.end()) {
			return iterator->second;
		}
		else {
			static const std::set<Bus*, BusPtrHasher> empty_set;
			return empty_set;
		}
	}

	size_t TransportCatalogue::GetDistanceDirectly(Stop* from, Stop* to) const{
		if (distances_.count(std::make_pair(from, to)) > 0) {
			return distances_.at(std::make_pair(from, to));
		}
		else {
			return 0U;
		}
	}

	size_t TransportCatalogue::GetDistance(Stop* from, Stop* to) const {
		size_t result = GetDistanceDirectly(from, to);
		return (result > 0 ? result : GetDistanceDirectly(to, from));
	}

	BusInfo TransportCatalogue::GetBusInfo(const Bus* bus) const {

		BusInfo bus_info;

		bus_info.stops_count_ = bus->stops_.size();

		std::set<const Stop*> uniq_stops(bus->stops_.begin(), bus->stops_.end());
		bus_info.unique_stops_count_ = uniq_stops.size();

		for (size_t i = 1; i < bus->stops_.size(); ++i) {
			Stop* prev_stop = const_cast<Stop*>(bus->stops_[i - 1]);
			Stop* curr_stop = const_cast<Stop*>(bus->stops_[i]);

			bus_info.route_length_ += GetDistance(prev_stop, curr_stop);
			bus_info.geo_route_length_ += transport::geo::ComputeDistance(prev_stop->coordinate_, curr_stop->coordinate_);
		}

		bus_info.curvature_ = bus_info.route_length_ / bus_info.geo_route_length_;

		return bus_info;
	}



}
