// Passenger logics

#ifndef VIBE_CODING_PASSENGERS_H
#define VIBE_CODING_PASSENGERS_H

#include <iostream>
#include <string>
#include <cstdint>

class Passenger {
public:
    Passenger(const std::string&, int16_t);

    std::string getName() const;
    int16_t getDestination() const;

private:
    std::string name;
    int16_t destinationID;
};

#endif //VIBE_CODING_PASSENGERS_H
