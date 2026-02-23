cmake_minimum_required(VERSION 4.0)
project(Vibe_Coding)

set(CMAKE_CXX_STANDARD 11)

add_executable(Vibe_Coding "BusStopManager.cpp"
        backend/System.cpp
        backend/System.h
        backend/Bus.cpp
        backend/Bus.h
        backend/Passengers.cpp
        backend/Passengers.h
)
