#include "DataLoader.hpp"
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<std::vector<GeoPoint>> LoadMapData(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Не вдалося відкрити мапу за шляхом: " << path << std::endl;
        return {};
    }
    
    json j;
    file >> j;
    std::vector<std::vector<GeoPoint>> borders;
    
    for (const auto& feature : j["features"]) {
        auto geom = feature["geometry"];
        auto type = geom["type"];
        
        if (type == "Polygon") {
            for (const auto& ring : geom["coordinates"]) {
                std::vector<GeoPoint> poly;
                for (const auto& c : ring)
                    poly.push_back({c[0].get<float>(), c[1].get<float>()});
                borders.push_back(poly);
            }
        } else if (type == "MultiPolygon") {
            for (const auto& poly_set : geom["coordinates"]) {
                for (const auto& ring : poly_set) {
                    std::vector<GeoPoint> poly;
                    for (const auto& c : ring)
                        poly.push_back({c[0].get<float>(), c[1].get<float>()});
                    borders.push_back(poly);
                }
            }
        }
    }
    return borders;
}
