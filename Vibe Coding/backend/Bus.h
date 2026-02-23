// The core of bus logic.

#ifndef VIBE_CODING_BUS_H
#define VIBE_CODING_BUS_H

#include "Passengers.h"

#include <fstream>
#include <vector>
#include <cstdint>
#include <sstream>
#include <algorithm>

struct GPS {
    double longitude;
    double latitude;
};

struct Stop {
    std::string Name;
    int16_t ID;
    GPS coord;
};

struct BusState {
    int16_t currentstopID;
    bool AtNewStop;
};

class Bus {
public:
    explicit Bus(std::ifstream &);
    ~Bus() = default;

    //Passenger Managements
    void createpassenger(const std::string & name, int16_t destinationID);
    std::vector <std::string> UnloadPassengers();

    //Stop Register Management
    void RegisterStop(int16_t);
    void DeletePastStop(int16_t);
    void SortDestinationsByCurrentStop();
    int16_t SearchStop(int16_t) const;
    std::string GetStopName(int16_t) const;
    int16_t GetSTOPNUM() const;
    const std::vector<Stop>& GetStoplst() const;
    const std::vector<int16_t>& GetDestination() const;
    void ClearDestination();


    //Bus State Management
    void SetAtNewStop(bool);
    bool IsAtNewStop() const;
    int16_t GetCurrentstopID() const;

    //Movements
    void GoToNextStop();
    void ReturnToPreviousStop();
    void SetStop(int16_t);

private:
    //constant
    std::string BUSLINE;
    int16_t STOPNUM;
    std::vector<Stop> Stoplst;

    //variable
    std::vector <int16_t> Destinations;
    std::vector <Passenger> Passengerlst;
    BusState State;
};

#endif //VIBE_CODING_BUS_H
