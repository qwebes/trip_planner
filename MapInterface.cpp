#include "MapInterface.hpp"
#include <cmath>

GeoPoint MapInterface::screenToGeo(int x, int y, int width, int height) {
    GeoPoint p;
    p.lon = (float)(x - 5) / (width - 10) * (max_lon - min_lon) + min_lon;
    p.lat = max_lat - (float)(y - 5) / (height - 10) * (max_lat - min_lat);
    return p;
}

std::string MapInterface::findNearestStation(GeoPoint p, const std::map<std::string, GeoPoint>& cityCoords) {
    std::string bestName = "";
    double minDist = 1e9;

    for (const auto& [name, coord] : cityCoords) {
        double dist = std::sqrt(std::pow(p.lon - coord.lon, 2) + std::pow(p.lat - coord.lat, 2));
        if (dist < minDist) {
            minDist = dist;
            bestName = name;
        }
    }
    return bestName;
}
