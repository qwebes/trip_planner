#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>

#include "DataLoader.hpp"
#include "TripSolver.hpp"
#include "MapInterface.hpp"
#include "Const.hpp"

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>

using namespace ftxui;

int main() {
    AppState state;
    std::map<std::string, GeoPoint> cityCoords;
    std::vector<std::string> cities;
    std::vector<Train> all_trains;
    MapInterface mapUI;
    
    std::ifstream cityFile(Config::cityFile);
        if (!cityFile.is_open()) {
            cities.push_back("Data not found");
            std::cerr << "File not found: " << Config::cityFile << std::endl;
        } else {
            std::string name; float lat, lon;
            while (cityFile >> name >> lat >> lon) {
                cities.push_back(name);
                cityCoords[name] = {lon, lat};
            }
        }
    

    auto borders = LoadMapData(Config::mapFile);
    
    auto checkbox_custom = Checkbox("Вказати точку на мапі", &mapUI.useCustomLocation);
    auto combo_from = Dropdown(&cities, &state.selected_from);
    auto combo_to = Dropdown(&cities, &state.selected_to);
    
    std::vector<std::string> priority_options = {"Швидкість", "Вартість"};
    auto priority_menu = Radiobox(&priority_options, &state.priority_selected);
      

    auto button = Button(&state.button_label, [&] {
        CalculateTrip(state, cityCoords, all_trains, cities, mapUI);
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
        
        int h_disp = (int)state.total_time;
        int m_disp = (int)((state.total_time - h_disp) * 60 + 0.5);
        
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
            if (state.click_state >= 1) {
                auto p_s = project(state.start_pt);
                c.DrawBlock(p_s.first, p_s.second, true, Color::Cyan);

                if (!state.finalPath.empty()) {
                    auto p_st = project(cityCoords[state.finalPath[0]]);
                    c.DrawPointLine(p_s.first, p_s.second, p_st.first, p_st.second, Color::Cyan);
                }
            }

            if (state.click_state == 2) {
                auto p_e = project(state.end_pt);
                c.DrawBlock(p_e.first, p_e.second, true, Color::Cyan);

                if (!state.finalPath.empty()) {
                    auto p_en = project(cityCoords[state.finalPath.back()]);
                    c.DrawPointLine(p_e.first, p_e.second, p_en.first, p_en.second, Color::Cyan);
                }
            }
        }

        if (!state.finalPath.empty()) {
            for (size_t i = 0; i < state.finalPath.size() - 1; ++i) {
                if (cityCoords.count(state.finalPath[i]) && cityCoords.count(state.finalPath[i+1])) {
                    auto p1 = project(cityCoords[state.finalPath[i]]);
                    auto p2 = project(cityCoords[state.finalPath[i+1]]);
                    c.DrawBlockLine(p1.first, p1.second, p2.first, p2.second, Color::RGB(193, 2, 2));
                }
            }
        }
        
        auto trip_steps = [&] {
            Elements steps;
            if (state.finalPath.empty()) return vbox({});
            steps.push_back(text(" " + cities[state.selected_from] + " ➔ " + cities[state.selected_to]) | bold | color(Color::Yellow));
            steps.push_back(separatorEmpty());
            steps.push_back(separator());
            auto format_time = [](int h, int m) {
                return std::to_string(h) + ":" + (m < 10 ? "0" : "") + std::to_string(m);
            };
            for (size_t i = 0; i < state.finalPath.size() - 1; ++i) {
                for (const auto& tr : all_trains) {
                    if (tr.getFrom() == state.finalPath[i] && tr.getTo() == state.finalPath[i+1]) {
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
            steps.push_back(text("Ціна: " + std::to_string((int)state.total_price) + " грн")),
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
                        text(state.click_state >= 1 ? " [ " + state.start_name + " ] " : " [ Оберіть на мапі ] ") | color(Color::Cyan),
                        (state.click_state >= 1 && !state.finalPath.empty() ? [&]{
                            Destination dStart = toDest("Start", state.start_pt);
                            Destination cityStart = toDest(state.start_name, cityCoords[state.start_name]);
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
                        text(state.click_state == 2 ? " [ " + state.end_name + " ] " : " [ Оберіть на мапі ] ") | color(Color::Cyan),
                        (state.click_state == 2 && !state.finalPath.empty() ? [&]{
                            Destination dEnd = toDest("End", state.end_pt);
                            Destination cityEnd = toDest(state.end_name, cityCoords[state.end_name]);
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
                if (!state.finalPath.empty()) {
                                return true;
                            }
                int mx = (mouse.x - 43) * 2;
                int my = (mouse.y - 3) * 4;
                GeoPoint p = mapUI.screenToGeo(mx, my, 160, 160);
                std::string nearest = mapUI.findNearestStation(p, cityCoords);
                if (state.click_state == 0 || state.click_state == 2) {
                    state.start_pt = p; state.start_name = nearest; state.click_state = 1;
                    for (int i = 0; i < (int)cities.size(); ++i)
                        if (cities[i] == nearest) state.selected_from = i;
                } else if (state.click_state == 1) {
                    state.end_pt = p; state.end_name = nearest; state.click_state = 2;
                    for (int i = 0; i < (int)cities.size(); ++i)
                        if (cities[i] == nearest) state.selected_to = i;
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



