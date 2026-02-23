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
#include <cstring>
#include <algorithm>
#include <mutex>
#include <termios.h>
#include <sys/ioctl.h>

#include "backend/Passengers.h"
#include "backend/Bus.h"
#include "backend/System.h"
#include "backend/Connector.h"
using namespace std;

const char* GPIO_SOCKET = "/tmp/gpio.sock";

const string BUSINFO_PATH = "./resources/";
const string BUSINFO_NAME = "StopList.txt";
const string PININFO_PATH = "./resources/";
const string PININFO_NAME = "PinInfo.txt";


int main() {
    system("x-terminal-emulator -e ./BusStopManager");

    ifstream businfo(BUSINFO_PATH + BUSINFO_NAME);
    ifstream pininfo(PININFO_PATH + PININFO_NAME);
    if (businfo.fail() || pininfo.fail()) {
        perror("Error loading resource file");
        return 1;
    }

    //Load Bus Information
    Bus bus(businfo);
    businfo.close();

    //Load Button -> Pin Layout
    int16_t pinID = 0;
    vector <int16_t> BUTTON_PIN;
    while (pininfo >> pinID) {
        BUTTON_PIN.push_back(pinID);
    }

    //GPIO link socket configuration
    unlink(UI_SOCKET_IN);
    int GPIO_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (GPIO_fd < 0) {
        perror("gpio reader socket");
        return 1;
    }
    sockaddr_un ui_addr{};
    ui_addr.sun_family = AF_UNIX;
    strcpy(ui_addr.sun_path, GPIO_SOCKET);
    if (bind(GPIO_fd, (sockaddr*)&ui_addr, sizeof(ui_addr)) < 0) {
        perror("gpio reader bind");
        return 1;
    }

    //Start GPIO receiver
    system("python3 ./GPIOreceiver.py &");

    //Create threads
    thread gpsThread(GPStracker, ref(bus));
    thread inputThread(Interface, ref(bus));
    thread registerThread(PassengersRegister, ref(bus), ref(BUTTON_PIN), GPIO_fd);

    gpsThread.join();
    inputThread.join();
    registerThread.join();
}

