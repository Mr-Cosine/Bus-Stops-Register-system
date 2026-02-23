// All functions that connects user to the internal data


#ifndef VIBE_CODING_SYSTEM_H
#define VIBE_CODING_SYSTEM_H

#include "Bus.h"

#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>
#include <fcntl.h>
#include <termios.h>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <mutex>

extern bool ENDINDICATOR;
constexpr double PI = 3.141592653589;

//Bus system
void ReportStop(std::ostream & , const Bus &);
void ShowStops(std::ostream & , const Bus &);

//Threads to be executed
void PassengersRegister(Bus&, std::vector <int16_t>, int);
void GPStracker(Bus &);
void Interface(Bus &);
void Connector(Bus &, int);

//GPS
int openUARTstream();
double haversine(GPS, GPS);
GPS readGPS(int);
std::string getGPSLine(int);
bool parseGNGGA(const std::string&, GPS&);

//GPIO
int16_t readGPIO(int);

void terminate();

#endif //VIBE_CODING_SYSTEM_H