#pragma once

#include "geo.h"

#include <deque>
#include <string>
#include <string_view>
#include <set>
#include <unordered_map>
#include <vector>
#include "geo.h"

namespace transport{

	struct Stop
	{
		Stop() = default;

		Stop(const std::string& name, const transport::geo::Coordinates& coordinate) :
			name_(name), coordinate_(coordinate) {}

		std::string name_;
		transport::geo::Coordinates coordinate_;
		std::set<std::string_view> buses_by_stop_;
	};

	struct Bus
	{
		Bus() = default;

		Bus(const std::string& name, const std::vector<const Stop*>& stops) :
			name_(name), stops_(stops) {
		}

		bool operator<(Bus& other) {
			return std::lexicographical_compare(name_.begin(), name_.end(),
				other.name_.begin(), other.name_.end());
		}

		std::string name_;
		std::vector<const Stop*> stops_;
	};

	struct BusInfo {
		BusInfo() = default;

		BusInfo(size_t stops_count, size_t unique_stops_count, double route_length, double curvature) :
			stops_count_(stops_count), unique_stops_count_(unique_stops_count), 
			route_length_(route_length), curvature_(curvature) {}

		size_t stops_count_ = 0u;
		size_t unique_stops_count_ = 0u;
		double geo_route_length_ = 0.0;
		double route_length_ = 0.0;
		double curvature_ = 0.0;
	};

	struct BusPtrHasher {
		bool operator()(Bus* lhs, Bus* rhs) const {
			return *lhs < *rhs;
		};
	};

	struct PairHasher {
		std::size_t operator()(const std::pair<Stop*, Stop*>& pair) const {
			return std::hash<const void*>{}(static_cast<const void*>(pair.first)) * 17
				+ std::hash<const void*>{}(static_cast<const void*>(pair.second));
		}
	};

	class TransportCatalogue {
	public:

		void AddStop(Stop&& stop);
		void AddBus(Bus&& bus);
		void SetDistance(Stop* from, Stop* to, size_t distance);

		const Stop* GetStop(std::string_view name) const;
		const Bus* GetBus(std::string_view name) const;

		const std::set<Bus*, BusPtrHasher>& GetBusesByStop(std::string_view name) const;

		size_t GetDistanceDirectly(Stop* from, Stop* to) const;
		size_t GetDistance(Stop* from, Stop* to) const;
		BusInfo GetBusInfo(const Bus* bus) const;

	private:

		std::deque<Stop> stops_;
		std::deque<Bus> buses_;

		std::unordered_map<std::string_view, Stop* > stopname_to_stop_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;

		std::unordered_map<std::string_view, std::set<Bus*, BusPtrHasher> > stop_to_buses_;
		std::unordered_map<std::pair<Stop*, Stop*>, size_t, PairHasher> distances_;

	};

}
