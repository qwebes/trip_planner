#ifndef DATALOADER_HPP
#define DATALOADER_HPP

#include <vector>
#include <string>
#include "MapInterface.hpp" 

std::vector<std::vector<GeoPoint>> LoadMapData(const std::string& path);

#endif // !DATALOADER_HPP
