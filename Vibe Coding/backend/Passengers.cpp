// Passenger logics

#include "Passengers.h"

Passenger::Passenger(const std::string &psngrname, const int16_t destinationID) :
    name(psngrname), destinationID(destinationID)
{}

std::string Passenger::getName() const{
    return name;
}

int16_t Passenger::getDestination() const{
    return destinationID;
}
