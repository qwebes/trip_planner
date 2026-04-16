#ifndef TRIPSOLVER_HPP
#define TRIPSOLVER_HPP

#include <vector>
#include <string>
#include <map>
#include "MapInterface.hpp"
#include "Train.hpp"
#include "Graph.hpp"
#include "Destination.hpp"

struct AppState {

    std::vector<std::string> finalPath;
    double total_price = 0.0;
    double total_time = 0.0;
    
    GeoPoint start_pt = {0, 0};
    GeoPoint end_pt = {0, 0};
    std::string start_name = "";
    std::string end_name = "";
    int click_state = 0;
    
    int selected_from = 0;
    int selected_to = 0;
    int priority_selected = 0;
    std::string button_label = "Розрахувати маршрут";
};

Destination toDest(std::string name, GeoPoint p);


void CalculateTrip(
    AppState& state,
    std::map<std::string, GeoPoint>& cityCoords,
    std::vector<Train>& all_trains,
    const std::vector<std::string>& cities,
    MapInterface& mapUI
);

#endif // !TRIPSOLVER_HPP
