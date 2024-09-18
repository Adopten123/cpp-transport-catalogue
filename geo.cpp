#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

namespace transport {
    namespace geo {

        Coordinates::Coordinates(double latitude, double longitude) :
            lat(latitude), lng(longitude) {}

        bool Coordinates::operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }

        bool Coordinates::operator!=(const Coordinates& other) const {
            return !(*this == other);
        }

        double ComputeDistance(Coordinates from, Coordinates to) {
            using namespace std;
            if (from == to) {
                return 0;
            }

            static const double dr = 3.1415926535 / 180.;
            static const int earth_radius = 6371000;

            return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
                * earth_radius;
        }

        std::size_t CoordinateHasher::operator()(const Coordinates& coords) const
        {
            return static_cast<std::size_t>(coords.lat + 17 * coords.lng);
        }

    }  // namespace geo

} //namesapce transport