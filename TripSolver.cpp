#include "TripSolver.hpp"
#include "Const.hpp"

#include <fstream>

Destination toDest(std::string name, GeoPoint p) {
    Destination d;
    d.setName(name);
    d.setLat(p.lat);
    d.setLon(p.lon);
    return d;
}

void CalculateTrip(
    AppState& state,
    std::map<std::string, GeoPoint>& cityCoords,
    std::vector<Train>& all_trains,
    const std::vector<std::string>& cities,
    MapInterface& mapUI
) {
    if (!state.finalPath.empty()) {
        state.finalPath.clear();
        state.click_state = 0;
        state.total_price = 0.0;
        state.total_time = 0.0;
        state.start_name = "";
        state.end_name = "";
        state.button_label = "Розрахувати маршрут";
    } else {
        all_trains.clear();
        Graph tripGraph;
        std::ifstream trainFile(Config::trainFile);
        Train t;
        std::map<std::pair<std::string, std::string>, bool> hasTrain;
        
        while (t.load(trainFile)) {
            all_trains.push_back(t);
            hasTrain[{t.getFrom(), t.getTo()}] = true;
        
            double weight;
            if (state.priority_selected == 1) {
                weight = t.getPrice();
            } else {
                weight = (double)t.getDuration();
            }

            tripGraph.addEdge(t.getFrom(), t.getTo(), weight);
        }
        
        double fuelPrice = 98.0, consumption = 0.08;
        double pricePerKm = fuelPrice * consumption;

        state.total_price = 0;
        state.total_time = 0;
    
        for (auto const& [n1, c1] : cityCoords) {
            for (auto const& [n2, c2] : cityCoords) {
                if (n1 == n2 || hasTrain[{n1, n2}]){
                    continue;
                }
            
                Destination city1 = toDest(n1, c1), city2 = toDest(n2, c2);
                double dist = city1.distanceTo(city2);
                double weight;

                if (state.priority_selected == 1) {
                    weight = dist * pricePerKm;
                } else {
                    weight = (dist / 70.0) * 60.0;
                }
                tripGraph.addEdge(n1, n2, weight);
            }
        }

        state.finalPath = tripGraph.findShortestPath(cities[state.selected_from], cities[state.selected_to]);


         if (!state.finalPath.empty()) {
             for (size_t i = 0; i < state.finalPath.size() - 1; ++i) {
                 bool found = false;
                 for (const auto& tr : all_trains) {
                     if (tr.getFrom() == state.finalPath[i] && tr.getTo() == state.finalPath[i+1]) {
                         state.total_price += tr.getPrice();
                         state.total_time += (tr.getDuration() / 60.0);
                         found = true;
                         break;
                     }
                 }
                 if (!found) {
                     Destination c1 = toDest(state.finalPath[i], cityCoords[state.finalPath[i]]);
                     Destination c2 = toDest(state.finalPath[i+1], cityCoords[state.finalPath[i+1]]);
                     double d = c1.distanceTo(c2);
                     state.total_price += (d * pricePerKm);
                     state.total_time += (d / 80.0);
                 }
             }
                

                            
                            
            
         if (mapUI.useCustomLocation) {
             Destination dStart = toDest("StartPoint", state.start_pt);
             Destination dEnd = toDest("EndPoint", state.end_pt);
             if (state.click_state >= 1) {
                 Destination firstCity = toDest(state.start_name, cityCoords[state.start_name]);
                 double d1 = dStart.distanceTo(firstCity);
                 
                 state.total_time += (d1 < 25.0) ? (d1 / 5.0) : (d1 / 50.0);
                 if (d1 >= 1.0) {
                     state.total_price += (d1 * pricePerKm);
                 }
             }

             if (state.click_state == 2) {
                 Destination lastCity = toDest(state.end_name, cityCoords[state.end_name]);
                 double d2 = lastCity.distanceTo(dEnd);
                 
                 state.total_time += (d2 < 25.0) ? (d2 / 5.0) : (d2 / 50.0);
                 if (d2 >= 1.0) {
                     state.total_price += (d2 * pricePerKm);
                 }
             }
         }
                 

             state.button_label = "Вибрати новий маршрут";

            }
        }

}
