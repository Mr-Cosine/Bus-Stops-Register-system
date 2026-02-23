// All functions that connects user to the internal data

#include "System.h"
#include "Bus.h"
#include "Connector.h"

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

std::mutex stateMutex;
//bool ENDINDICATOR = false;

void ReportStop(std::ostream &out, Bus &bus) {
    out << "Reporting Stops:\n";
    const std::vector <int16_t>& Destinations = bus.GetDestination();
    const std::vector <Stop>& Stoplst = bus.GetStoplst();
    if (Destinations.empty()) {
        out << "No destinations registered.\n";
        return;
    }

    const int nextstop = Destinations[0];
    if (nextstop < 0 || nextstop >= bus.GetSTOPNUM()) {
        out << "Invalid destination appeared.\n";
        bus.ClearDestination();
        return;
    }

    out << "The next stop required is: " << "\n" << Stoplst[nextstop].Name << "\n\n";
    out << "Future Stops:\n";
    for (const int16_t destID : Destinations) {
        if (destID != nextstop) {
            out << "\t" << Stoplst[destID].Name << "\n";
        }
    }
}

void ShowStops(std::ostream &out, const Bus & bus) {
    const std::vector <Stop>& Stoplst = bus.GetStoplst();
    for (const Stop &stop : Stoplst) {
        out << stop.Name << ": " << stop.ID << "\n";
    }
}

int16_t readGPIO(int gpio_fd) {
    char buffer[128];
    ssize_t len = recv(gpio_fd, buffer, sizeof(buffer) - 1, 0);

    if (len > 0) {
        buffer[len] = '\0';
        std::string event(buffer);

        size_t pos = event.find("BTN");

        if (pos != std::string::npos) return static_cast<int16_t>(std::stoi(event.substr(pos + 3)));
    }
    return -1;
}

int openUARTstream() {
    int uart = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
    fcntl(uart, F_SETFL, 0); // turn off non-blocking

    if (uart == -1) {
        std::cerr << "Failed to open UART" << "\n";
        return -1;
    }

    struct termios options;
    tcgetattr(uart, &options);

    cfsetispeed(&options, B9600);   // GPS default baud rate
    cfsetospeed(&options, B9600);

    options.c_cflag |= (CLOCAL | CREAD);   // enable receiver
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;                // 8 data bits
    options.c_cflag &= ~PARENB;            // no parity
    options.c_cflag &= ~CSTOPB;            // 1 stop bit
    options.c_cflag &= ~CRTSCTS;           // no flow control

    tcsetattr(uart, TCSANOW, &options);

    return uart;
}

GPS readGPS(int uart) {
    while (true) {
        std::string line = getGPSLine(uart);

        if (line.find("$GNGGA") == 0 || line.find("GPGGA") == 0) { //When valid GNGGA line comes
            GPS coord = {0,0};
            parseGNGGA(line, coord); // Parse NMEA

            return coord;
        }
    }
}

std::string getGPSLine(int uart) {  //Get NMEA line
    std::string line;
    char c;

    while (read(uart, &c, 1) == 1) {
        if (c == '\r') continue;
        if (c == '\n') break;
        line += c;
    }

    return line;
}

bool parseGNGGA(const std::string& sentence, GPS &coord) {
    std::vector<std::string> fields;
    std::stringstream ss(sentence);
    std::string item;

    while (std::getline(ss, item, ',')) {
        fields.push_back(item);
    }

    if (fields.size() < 6)
        return false;

    // Extract raw values
    std::string rawLat = fields[2];
    std::string rawNS  = fields[3];
    std::string rawLon = fields[4];
    std::string rawEW  = fields[5];

    if (rawLat.empty() || rawLon.empty())
        return false;

    // Convert latitude ddmm.mmmm → decimal degrees
    double lat = std::stod(rawLat);
    int latDeg = static_cast<int>(lat / 100);
    double latMin = lat - (latDeg * 100);
    lat = latDeg + (latMin / 60.0);

    if (rawNS == "S")
        lat = -lat;

    // Convert longitude dddmm.mmmm → decimal degrees
    double lon = std::stod(rawLon);
    int lonDeg = static_cast<int>(lon / 100);
    double lonMin = lon - (lonDeg * 100);
    lon = lonDeg + (lonMin / 60.0);

    if (rawEW == "W")
        lon = -lon;

    coord.latitude = lat;
    coord.longitude = lon;

    return true;
}

double haversine(GPS coord1, GPS coord2) {
    constexpr double EARTH_R = 6371000.0; // meters

    double Lat1 = coord1.latitude * PI / 180.0;
    double Lat2 = coord2.latitude * PI / 180.0;
    double Lon1 = coord1.longitude * PI / 180.0;
    double Lon2 = coord2.longitude * PI / 180.0;

    double dLat = Lat2- Lat1;
    double dLon = Lon2 - Lon1;

    double a = sin(dLat/2) * sin(dLat/2) + sin(dLon/2) * sin(dLon/2) * cos(Lat1) * cos(Lat2);

    return 2 * EARTH_R * atan2(sqrt(a), sqrt(1 - a));
}

void terminate() {
    std::cout << "Program terminated by user." << "\n";
    ENDINDICATOR = true;
}

void Interface(Bus &bus) {
    while (!ENDINDICATOR) {
        if (bus.IsAtNewStop()){
            //Refresh terminal
            std::cout << "\033[2J\033[H";
            //Report this stop
            std::cout << "You have reached the stop: "
                 << bus.GetStopName(bus.GetCurrentstopID()) << "\n";

            bus.DeletePastStop(bus.GetCurrentstopID());

            //Unload passengers getting off here
            std::vector <std::string> unloadedpassenger = bus.UnloadPassengers();
            std::cout << "Dear ";
            for (const std::string & name : unloadedpassenger) {
                std::cout << name << ", ";
            }
            std::cout << "you may get off at this stop." << "\n";

            //Show upcoming destinations
            ReportStop(std::cout, bus);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void GPStracker(Bus &bus) {
    constexpr double THRESHOLDDIST = 30.0; // meters
    int uart = openUARTstream();
    while (!ENDINDICATOR) {
        // Read GPS (replace with your real GPS function)
        GPS currentcoord = readGPS(uart);

        // Check distance to each stop
        int16_t detectedStop = -1;
        const std::vector<Stop>& Stoplst = bus.GetStoplst();
        for (const auto& stop : Stoplst) {
            double dist = haversine(currentcoord, stop.coord);
            if (dist <= THRESHOLDDIST) {
                detectedStop = stop.ID;
                break;
            }
        }

        // Update shared state
        {
            std::lock_guard<std::mutex> lock(stateMutex);
            if (detectedStop != -1 && detectedStop != bus.GetCurrentstopID()) {
                bus.SetAtNewStop(true);
                bus.SetStop(detectedStop);
            }
            else {
                bus.SetAtNewStop(false);
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void PassengersRegister(Bus& bus, const std::vector <int16_t> &BUTTON_PIN, int sockfd) {
    using namespace std::chrono;
    auto lastPressTime = steady_clock::now();

    // Main loop
    while (!ENDINDICATOR) {
        int16_t ButtonID = readGPIO(sockfd);

        if (ButtonID >= 0) {
            auto now = steady_clock::now();
            auto diff = duration_cast<milliseconds>(now - lastPressTime);

            if (diff.count() > 500) {
                int16_t SelectedStop = 0;
                int16_t stopID = 0;
                for (int16_t PinID : BUTTON_PIN) {
                    if (PinID == ButtonID) SelectedStop = stopID;
                    stopID ++;
                }
                std::string name;
                std::cout << "\nPassenger button pressed for stop "
                     << SelectedStop << ". Enter initials: ";
                std::cin >> name;

                bus.RegisterStop(SelectedStop);
                bus.createpassenger(name, SelectedStop);

                lastPressTime = now;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}