#ifndef Destination_hpp
#define Destination_hpp

#include <string>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

class Destination {
private:
    string name;
    double latitude;
    double longitude;

public:
    Destination() : name(""), latitude(0), longitude(0) {}
    
    bool load(ifstream& file);
    
    void setName(const string& newName);
    void setLat(double lat);
    void setLon(double lon);
    
    string getName() const { return name; }
    double getLat() const { return latitude; }
    double getLon() const { return longitude; }
    
    double distanceTo(const Destination& d) const;

    static double calculateDistance(const Destination& d1, const Destination& d2);
};

#endif // !Destination_hpp
