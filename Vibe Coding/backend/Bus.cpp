// The core of bus logic.

#include "Bus.h"
#include "Passengers.h"

#include <fstream>
#include <vector>
#include <cstdint>
#include <sstream>
#include <algorithm>

Bus::Bus(std::ifstream &businfo) :
    State({0, false}) {  //constructor: initialize based on the ID given(if needed) and Names of Stops
    Passengerlst.clear();
    Stoplst.clear();

    getline(businfo, BUSLINE);

    std::string line;
    while (std::getline(businfo, line)) {
        std::stringstream linebuffer(line);
        std::string word;

        std::string stopname;
        double longitude;
        double latitude;

        while (linebuffer >> word && word != "/") {
            stopname += word + " ";
        }
        if (!stopname.empty() && stopname.back() == ' ') stopname.pop_back();

        linebuffer >> longitude;
        linebuffer >> latitude;

        Stoplst.push_back({stopname, State.currentstopID, {longitude, latitude}});
        State.currentstopID++;
    }
    State.currentstopID = 0;
    STOPNUM = static_cast<int16_t> (Stoplst.size());
}

void Bus::RegisterStop(const int16_t inputID) {    //User input desired stops (translated to the given stopID
    const int16_t NewDestinationID = SearchStop(inputID);   //check if stop is already existing
    if (NewDestinationID != -1) {
        return;
    }

    for (const Stop &stop : Stoplst) {  //Put the stop into the list
        if (stop.ID == inputID) {
            Destinations.push_back(inputID);
            return;
        }
    }
}

void Bus::DeletePastStop(const int16_t stopID) { //after one stop, delete it from the registered destinations
    const int index = SearchStop(stopID);   // find the index of this stop in the vector

    if (index != -1) {
        Destinations.erase(Destinations.begin() + index);
    }
}

void Bus::SortDestinationsByCurrentStop() {
    std::sort(Destinations.begin(), Destinations.end(),[this](const int a, const int b) {
        const int da = (a - State.currentstopID + STOPNUM) % STOPNUM;
        const int db = (b - State.currentstopID + STOPNUM) % STOPNUM;
        return da < db;
    });
}

std::string Bus::GetStopName(const int16_t StopID) const{
    if (StopID >= 0 && StopID < STOPNUM) return Stoplst[StopID].Name;
    return "";
}

int16_t Bus::GetCurrentstopID() const {
    return State.currentstopID;
}

void Bus::createpassenger(const std::string & name, int16_t destinationID) {    //register a passenger
    Passengerlst.emplace_back(name, destinationID);
}

std::vector <std::string> Bus::UnloadPassengers() { //delete passengers that ought to get off at a stop
    std::vector <std::string> namelst;
    for (auto it = Passengerlst.begin(); it != Passengerlst.end();) {
        if (it->getDestination() == State.currentstopID) {
            it = Passengerlst.erase(it);
            namelst.push_back(it->getName());
        }
        else {
            ++it;
        }
    }
    return namelst;
}

int16_t Bus::SearchStop(const int16_t desireID) const{  //Search stop in the destination registered, return -1 if no find
    int16_t index = 0;
    for (const int16_t destID : Destinations) {
        if (destID == desireID) {
            return index;
        }
        index++;
    }
    return -1;
}

void Bus::GoToNextStop() {
    State.currentstopID ++;

    if (State.currentstopID >= STOPNUM) {
        State.currentstopID = 0;
    }
}

void Bus::ReturnToPreviousStop() {
    State.currentstopID --;

    if (--State.currentstopID < 0) {
        State.currentstopID = static_cast <int16_t> (STOPNUM - 1);
    }
}

void Bus::SetStop(const int16_t newstopID) {  //Update State.currentstopID
    if (newstopID >= 0 && newstopID < STOPNUM) {
        State.currentstopID = newstopID;
    }
}

void Bus::SetAtNewStop(bool state) {
    State.AtNewStop = state;
}
bool Bus::IsAtNewStop() const{
    return State.AtNewStop;
}

int16_t Bus::GetSTOPNUM() const {
    return STOPNUM;
}

const std::vector<Stop>& Bus::GetStoplst() const{
    return Stoplst;
}

const std::vector <int16_t>& Bus::GetDestination() const{
    return Destinations;
}

void Bus::ClearDestination() {
    Destinations.clear();
    Passengerlst.clear();
}