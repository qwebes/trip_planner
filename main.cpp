#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>

#include "Destination.hpp"
#include "Train.hpp"
#include "Graph.hpp"
#include "MapInterface.hpp"


#include <nlohmann/json.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>

using json = nlohmann::json;
using namespace ftxui;

Destination toDest(std::string name, GeoPoint p) {
    Destination d;
    d.setName(name);
    d.setLat(p.lat);
    d.setLon(p.lon);
    return d;
}

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
                for (const auto& c : ring) poly.push_back({c[0].get<float>(), c[1].get<float>()});
                borders.push_back(poly);
            }
        } else if (type == "MultiPolygon") {
            for (const auto& poly_set : geom["coordinates"]) {
                for (const auto& ring : poly_set) {
                    std::vector<GeoPoint> poly;
                    for (const auto& c : ring) poly.push_back({c[0].get<float>(), c[1].get<float>()});
                    borders.push_back(poly);
                }
            }
        }
    }
    return borders;
}


int main() {
    std::map<std::string, GeoPoint> cityCoords;
    std::vector<std::string> cities;

    std::ifstream cityFile("/Users/virapoberezhnik/Desktop/trip_planner/build/Debug/destination.txt");
    Destination d;
    while (d.load(cityFile)) {
        cityCoords[d.getName()] = { (float)d.getLon(), (float)d.getLat() };
        cities.push_back(d.getName());
    }

    if (cities.empty()) {
        std::cerr << "Помилка destination.txt" << std::endl;
    }

    int selected_from = 0;
    int selected_to = (cities.size() > 1) ? 1 : 0;
    int priority_selected = 0;
    double total_price = 0.0;
    double total_time = 0.0;
    std::vector<std::string> priority_options = {"Швидкість", "Вартість"};
    std::vector<std::string> finalPath;
    std::vector<Train> all_trains;

    MapInterface mapUI;
    auto borders = LoadMapData("/Users/virapoberezhnik/Desktop/trip_planner/ukraine_geojson/UA_FULL_Ukraine.geojson");

    auto checkbox_custom = Checkbox("Вказати точку на мапі", &mapUI.useCustomLocation);
    auto combo_from = Dropdown(&cities, &selected_from);
    auto combo_to = Dropdown(&cities, &selected_to);
    auto priority_menu = Radiobox(&priority_options, &priority_selected);
    
    int click_state = 0;
    GeoPoint start_pt = {0, 0};
    GeoPoint end_pt = {0, 0};
    std::string start_name = "";
    std::string end_name = "";
    
    std::string button_label = "Розрахувати маршрут";
    
    auto button = Button(&button_label, [
        &finalPath, &total_price, &total_time, &start_pt, &end_pt,
        &start_name, &end_name, &cityCoords, &all_trains, &priority_selected,
        &button_label, &click_state, &cities, &selected_from, &selected_to, &mapUI
    ] {
        if (!finalPath.empty()) {
            finalPath.clear();
            click_state = 0;
            total_price = 0.0;
            total_time = 0.0;
            start_name = "";
            end_name = "";
            button_label = "Розрахувати маршрут";
        } else {
            all_trains.clear();
            Graph tripGraph;
            std::ifstream trainFile("train.txt");
            Train t;
            std::map<std::pair<std::string, std::string>, bool> hasTrain;
            
            while (t.load(trainFile)) {
                all_trains.push_back(t);
                hasTrain[{t.getFrom(), t.getTo()}] = true;
                double weight;
                if (priority_selected == 1) {
                    weight = t.getPrice();
                } else {
                    weight = (double)t.getDuration();
                }
                tripGraph.addEdge(t.getFrom(), t.getTo(), weight);
            }
            
            double fuelPrice = 98.0, consumption = 0.08;
            double pricePerKm = fuelPrice * consumption;

            for (auto const& [n1, c1] : cityCoords) {
                for (auto const& [n2, c2] : cityCoords) {
                    if (n1 == n2 || hasTrain[{n1, n2}]){
                        continue;
                    }
                
                    Destination city1 = toDest(n1, c1), city2 = toDest(n2, c2);
                    double dist = city1.distanceTo(city2);
                    double weight;

                    if (priority_selected == 1) {
                        weight = dist * pricePerKm;
                    } else {
                        weight = (dist / 70.0) * 60.0;
                    }
                    tripGraph.addEdge(n1, n2, weight);
                }
            }

                
            finalPath = tripGraph.findShortestPath(cities[selected_from], cities[selected_to]);

                  
            total_price = 0;
            total_time = 0;
                         
             if (!finalPath.empty()) {
                 for (size_t i = 0; i < finalPath.size() - 1; ++i) {
                     bool found = false;
                     for (const auto& tr : all_trains) {
                         if (tr.getFrom() == finalPath[i] && tr.getTo() == finalPath[i+1]) {
                             total_price += tr.getPrice();
                             total_time += (tr.getDuration() / 60.0);
                             found = true;
                             break;
                         }
                     }
                     if (!found) {
                         Destination c1 = toDest(finalPath[i], cityCoords[finalPath[i]]);
                         Destination c2 = toDest(finalPath[i+1], cityCoords[finalPath[i+1]]);
                         double d = c1.distanceTo(c2);
                         total_price += (d * pricePerKm); total_time += (d / 80.0);
                     }
                 }
                    
                        // finalPath = tripGraph.findShortestPath(cities[selected_from], cities[selected_to]);

                                    
                                    
                    
                 if (mapUI.useCustomLocation) {
                     Destination dStart = toDest("StartPoint", start_pt);
                     Destination dEnd = toDest("EndPoint", end_pt);
                     if (click_state >= 1) {
                         Destination firstCity = toDest(start_name, cityCoords[start_name]);
                         double d1 = dStart.distanceTo(firstCity);
                         
                         total_time += (d1 < 25.0) ? (d1 / 5.0) : (d1 / 50.0);
                         if (d1 >= 1.0) {
                             total_price += (d1 * pricePerKm);
                         }
                     }

                     if (click_state == 2) {
                         Destination lastCity = toDest(end_name, cityCoords[end_name]);
                         double d2 = lastCity.distanceTo(dEnd);
                         
                         total_time += (d2 < 25.0) ? (d2 / 5.0) : (d2 / 50.0);
                         if (d2 >= 1.0) total_price += (d2 * pricePerKm);
                     }
                 }
                         
                button_label = "Вибрати новий маршрут";
        
                    }
                }
            });

    auto controls = Container::Vertical({
        checkbox_custom,
        combo_from,
        combo_to,
        priority_menu,
        button,
    });
    
    auto renderer = Renderer(Container::Vertical({checkbox_custom, combo_from, combo_to, priority_menu, button}), [&] {
        int width = 80;
        int height = 40;
        auto c = Canvas(width * 2, height * 4);
        float min_lon = 22.1, max_lon = 40.2;
        float min_lat = 44.3, max_lat = 52.4;

        auto project = [&](GeoPoint p) {
            int x = (p.lon - min_lon) / (max_lon - min_lon) * (width * 2 - 10) + 5;
            int y = (max_lat - p.lat) / (max_lat - min_lat) * (height * 4 - 10) + 5;
            return std::make_pair(x, y);
        };
        
        int h_disp = (int)total_time;
        int m_disp = (int)((total_time - h_disp) * 60 + 0.5);
        
        for (const auto& poly : borders) {
            for (size_t i = 0; i < poly.size() - 1; ++i) {
                auto p1 = project(poly[i]);
                auto p2 = project(poly[i+1]);
                c.DrawPointLine(p1.first, p1.second, p2.first, p2.second, Color::White);
            }
        }

        for (const auto& [name, coord] : cityCoords) {
            auto p = project(coord);
            c.DrawBlock(p.first, p.second, true, Color::Yellow);
        }

        if (mapUI.useCustomLocation) {
            if (click_state >= 1) {
                auto p_s = project(start_pt);
                c.DrawBlock(p_s.first, p_s.second, true, Color::Cyan);

                if (!finalPath.empty()) {
                    auto p_st = project(cityCoords[finalPath[0]]);
                    c.DrawPointLine(p_s.first, p_s.second, p_st.first, p_st.second, Color::Cyan);
                }
            }

            if (click_state == 2) {
                auto p_e = project(end_pt);
                c.DrawBlock(p_e.first, p_e.second, true, Color::Cyan);

                if (!finalPath.empty()) {
                    auto p_en = project(cityCoords[finalPath.back()]);
                    c.DrawPointLine(p_e.first, p_e.second, p_en.first, p_en.second, Color::Cyan);
                }
            }
        }

        if (!finalPath.empty()) {
            for (size_t i = 0; i < finalPath.size() - 1; ++i) {
                if (cityCoords.count(finalPath[i]) && cityCoords.count(finalPath[i+1])) {
                    auto p1 = project(cityCoords[finalPath[i]]);
                    auto p2 = project(cityCoords[finalPath[i+1]]);
                    c.DrawBlockLine(p1.first, p1.second, p2.first, p2.second, Color::RGB(193, 2, 2));
                }
            }
        }
        
        auto trip_steps = [&] {
            Elements steps;
            if (finalPath.empty()) return vbox({});
            steps.push_back(text(" " + cities[selected_from] + " ➔ " + cities[selected_to]) | bold | color(Color::Yellow));
            steps.push_back(separatorEmpty());
            steps.push_back(separator());
            auto format_time = [](int h, int m) {
                return std::to_string(h) + ":" + (m < 10 ? "0" : "") + std::to_string(m);
            };
            for (size_t i = 0; i < finalPath.size() - 1; ++i) {
                for (const auto& tr : all_trains) {
                    if (tr.getFrom() == finalPath[i] && tr.getTo() == finalPath[i+1]) {
                        int dep_total = tr.getDepartureTotalMinutes();
                        int arr_total = dep_total + tr.getDuration();
                        int arr_h = (arr_total / 60) % 24;
                        int arr_m = arr_total % 60;
                        steps.push_back(hbox({
                            text(" " + tr.getFrom() + " ➔ " + tr.getTo()) | flex,
                            text(" " + format_time(tr.getDepartureHour(), tr.getDepartureMin()) + " | " + format_time(arr_h, arr_m)),
                            text(" | " + std::to_string((int)tr.getPrice()) + "₴ ")
                        }));
                        break;
                    }
                }
            }
            steps.push_back(separatorEmpty()),
            steps.push_back(text("Ціна: " + std::to_string((int)total_price) + " грн")),
            steps.push_back(text("Час: " + std::to_string(h_disp) + " год " + std::to_string(m_disp) + " хв"));
            return vbox(std::move(steps));
        };

        return hbox({
            vbox({
                text(" Налаштування") | bold | hcenter,
                separator(),
                checkbox_custom->Render(),
                separator(),
                text("Звідки:"),
                (mapUI.useCustomLocation ?
                    vbox({
                        text(click_state >= 1 ? " [ " + start_name + " ] " : " [ Оберіть на мапі ] ") | color(Color::Cyan),
                        (click_state >= 1 && !finalPath.empty() ? [
                            &start_pt, &start_name, &cityCoords, &finalPath
                        ]{
                            Destination dStart = toDest("Start", start_pt);
                            Destination cityStart = toDest(start_name, cityCoords[start_name]);
                            double dist = dStart.distanceTo(cityStart);
                            if (dist < 25.0) {
                                int mins = (int)(dist / 5.0 * 60.0);
                                return text("Пішки: " + std::to_string(mins) + " хв (" + std::to_string(dist).substr(0,3) + " км)") | color(Color::Cyan);
                            } else {
                                int mins = (int)(dist / 50.0 * 60.0);
                                return text("Машиною: " + std::to_string(mins) + " хв (" + std::to_string(dist).substr(0,4) + " км)") | color(Color::Cyan);
                            }
                        }() : text(""))
                    })
                    : combo_from->Render()),
                separatorEmpty(),
                text("Куди:"),
                (mapUI.useCustomLocation ?
                    vbox({
                        text(click_state == 2 ? " [ " + end_name + " ] " : " [ Оберіть на мапі ] ") | color(Color::Cyan),
                        (click_state == 2 && !finalPath.empty() ? [
                            &end_pt, &end_name, &cityCoords, &finalPath
                        ]{
                            Destination dEnd = toDest("End", end_pt);
                            Destination cityEnd = toDest(end_name, cityCoords[end_name]);
                            double dist = dEnd.distanceTo(cityEnd);
        
                            if (dist < 25.0) {
                                int mins = (int)(dist / 5.0 * 60.0);
                                return text("Пішки: " + std::to_string(mins) + " хв (" + std::to_string(dist).substr(0,3) + " км)") | color(Color::Cyan);
                            } else {
                                int mins = (int)(dist / 50.0 * 60.0);
                                return text("Машиною: " + std::to_string(mins) + " хв (" + std::to_string(dist).substr(0,4) + " км)") | color(Color::Cyan);
                            }
                        }() : text(""))
                    })
                    : combo_to->Render()),
                separatorEmpty(),
                priority_menu->Render(),
                separatorEmpty(),
                button->Render() | border | color(Color::Yellow) | hcenter,
                filler(),
                trip_steps(),
                separatorEmpty(),
            }) | size(WIDTH, EQUAL, 40) | border,
            vbox({
                text(" Trip planner") | hcenter | bold | color(Color::Yellow),
                separator(),
                canvas(std::move(c)) | flex
            }) | borderDouble | flex
        });
    });

    auto final_component = CatchEvent(renderer, [&](Event event) {
        if (mapUI.useCustomLocation && event.is_mouse()) {
            auto mouse = event.mouse();
            if (mouse.button == Mouse::Left && mouse.motion == Mouse::Pressed) {
                if (mouse.x <= 43) return false;
                if (!finalPath.empty()) {
                                return true;
                            }
                int mx = (mouse.x - 43) * 2;
                int my = (mouse.y - 3) * 4;
                GeoPoint p = mapUI.screenToGeo(mx, my, 160, 160);
                std::string nearest = mapUI.findNearestStation(p, cityCoords);
                if (click_state == 0 || click_state == 2) {
                    start_pt = p; start_name = nearest; click_state = 1;
                    for (int i = 0; i < (int)cities.size(); ++i)
                        if (cities[i] == nearest) selected_from = i;
                } else if (click_state == 1) {
                    end_pt = p; end_name = nearest; click_state = 2;
                    for (int i = 0; i < (int)cities.size(); ++i)
                        if (cities[i] == nearest) selected_to = i;
                }
                return true;
            }
        }
        return false;
    });

    auto screen = ScreenInteractive::TerminalOutput();
    screen.Loop(final_component);
    return 0;
}



