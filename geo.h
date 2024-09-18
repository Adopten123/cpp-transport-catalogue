#pragma once

#include <cmath>

namespace transport {
    namespace geo {

        struct Coordinates {

            Coordinates(double latitude, double longitude);

            double lat;
            double lng;

            bool operator==(const Coordinates& other) const;
            bool operator!=(const Coordinates& other) const;
        };

        double ComputeDistance(Coordinates from, Coordinates to);

        class CoordinateHasher {
        public:
            std::size_t operator()(const Coordinates& coords) const;
        };

    }
}