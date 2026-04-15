#ifndef Train_hpp
#define Train_hpp

#include <string>
#include <fstream>
#include <iostream>

using namespace std;

class Train {
private:
    string from;
    string to;
    int departureHour, departureMin;
    int duration;
    double price;

public:
    Train() : from(""), to(""), departureHour(0), departureMin(0), duration(0), price(0.0) {}

    bool load(ifstream& file);

    string getFrom() const {
        return from;
    }
    string getTo() const {
        return to;
    }
    int getDepartureHour() const {
            return departureHour;
    }
    int getDepartureMin() const {
            return departureMin;
    }
    int getDepartureTotalMinutes() const {
            return departureHour * 60 + departureMin;
    }
    
    int getDuration() const{
        return duration;
    }
    double getPrice() const {
        return price;
    }
};

#endif // !Train_hpp
