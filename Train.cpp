#include "Train.hpp"

bool Train::load(ifstream& file) {
    if (file >> from >> to >> departureHour >> departureMin >> duration >> price) {
        return true;
    }
    return false;
}


