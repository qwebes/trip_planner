cmake_minimum_required(VERSION 3.14)
project(trip_planner)

include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v5.0.0
)
FetchContent_MakeAvailable(ftxui)

add_executable(trip_planner
    main.cpp
    DataLoader.cpp DataLoader.hpp
    TripSolver.cpp TripSolver.hpp
    Graph.cpp Graph.hpp
    Train.cpp Train.hpp
    Destination.cpp Destination.hpp
    MapInterface.cpp MapInterface.hpp
    AppConfig.hpp
)

find_package(nlohmann_json REQUIRED)

target_link_libraries(trip_planner PRIVATE
    ftxui::screen
    ftxui::dom
    ftxui::component
    nlohmann_json::nlohmann_json
)
