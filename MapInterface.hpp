#ifndef MapInterface_hpp
#define MapInterface_hpp

#include <vector>
#include <string>
#include <map>
#include "Destination.hpp"

struct GeoPoint { float lon; float lat; };

class MapInterface {
public:
    GeoPoint clickedPoint = {0, 0};
    bool pointIsSet = false;
    std::string nearestStationName = "";
    bool useCustomLocation = false;

    const float min_lon = 22.1, max_lon = 40.2;
    const float min_lat = 44.3, max_lat = 52.4;

    MapInterface() {}

    std::string findNearestStation(GeoPoint p, const std::map<std::string, GeoPoint>& cityCoords);

    GeoPoint screenToGeo(int x, int y, int width, int height);
};

#endif // !MapInterface_hpp
