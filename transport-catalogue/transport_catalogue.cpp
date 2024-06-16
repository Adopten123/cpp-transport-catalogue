#include "transport_catalogue.h"

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

	BusInfo TransportCatalogue::GetBusInfo(const Bus* bus) const {
		size_t stop_count = bus->stops_.size();

		std::set<const Stop*> uniq_stops(bus->stops_.begin(), bus->stops_.end());
		size_t uniq_stop_count = std::move(uniq_stops).size();

		double length = 0;

		uint8_t unreal_crd_value = 255;

		transport::geo::Coordinates crd(unreal_crd_value, unreal_crd_value);

		for (auto stop : bus->stops_) {
			if (crd.lat == unreal_crd_value) {
				crd = stop->coordinate_;
			}
			else {
				length += transport::geo::ComputeDistance(crd, stop->coordinate_);
				crd = stop->coordinate_;
			}
		}

		return BusInfo(stop_count, uniq_stop_count, length);
	}

}
