#pragma once
#include <string>

namespace Config {
    const std::string cityFile = "/Users/virapoberezhnik/Desktop/trip_planner/destination.txt";
    const std::string mapFile = "/Users/virapoberezhnik/Desktop/trip_planner/ukraine_geojson/UA_FULL_Ukraine.geojson";
    const std::string trainFile = "/Users/virapoberezhnik/Desktop/trip_planner/build/Debug/train.txt";
    
    const double fuelPrice = 98.0;
    const double consumption = 0.08;
    const double pricePerKm = fuelPrice * consumption;
}
