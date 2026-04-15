#include "Destination.hpp"

bool Destination::load(ifstream& file) {
    if (file >> name >> latitude >> longitude) {
        return true;
    }
    return false;
}

void Destination::setName(const std::string& newName) {
    this->name = newName;
}

void Destination::setLat(double lat) {
    this->latitude = lat;
}

void Destination::setLon(double lon) {
    this->longitude = lon;
}

double Destination::calculateDistance(const Destination& d1, const Destination& d2) {
    const double PI = 3.14;
    const double R = 6371.0;

    auto toRad = [PI](double degree) {
        return degree * PI / 180.0;
    };

    double phi1 = toRad(d1.latitude);
    double phi2 = toRad(d2.latitude);
    double dLat = toRad(d2.latitude - d1.latitude);
    double dLon = toRad(d2.longitude - d1.longitude);

    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(phi1) * cos(phi2) *
               sin(dLon / 2) * sin(dLon / 2);
    
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
}

double Destination::distanceTo(const Destination& d) const {
    return Destination::calculateDistance(*this, d);
}
