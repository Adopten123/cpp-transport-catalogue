#pragma once

#include "domain.h"
#include "geo.h"


#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace transport{

	struct BusPtrHasher {
		bool operator()(domain::Bus* lhs, domain::Bus* rhs) const {
			return *lhs < *rhs;
		};
	};

	struct PairHasher {
		std::size_t operator()(const std::pair<domain::Stop*, domain::Stop*>& pair) const {
			return std::hash<const void*>{}(static_cast<const void*>(pair.first)) * 17
				+ std::hash<const void*>{}(static_cast<const void*>(pair.second));
		}
	};

	class TransportCatalogue {
	public:

		void AddStop(domain::Stop&& stop);
		void AddBus(domain::Bus&& bus);
		void SetDistance(domain::Stop* from, domain::Stop* to, size_t distance);

		const domain::Stop* GetStop(std::string_view name) const;
		const domain::Bus* GetBus(std::string_view name) const;

		const std::set<domain::Bus*, BusPtrHasher>& GetBusesByStop(std::string_view name) const;

		size_t GetDistanceDirectly(domain::Stop* from, domain::Stop* to) const;
		size_t GetDistance(domain::Stop* from, domain::Stop* to) const;
		domain::BusInfo GetBusInfo(const domain::Bus* bus) const;

	private:

		std::deque<domain::Stop> stops_;
		std::deque<domain::Bus> buses_;

		std::unordered_map<std::string_view, domain::Stop* > stopname_to_stop_;
		std::unordered_map<std::string_view, domain::Bus*> busname_to_bus_;

		std::unordered_map<std::string_view, std::set<domain::Bus*, BusPtrHasher> > stop_to_buses_;
		std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, size_t, PairHasher> distances_;

	};

}
