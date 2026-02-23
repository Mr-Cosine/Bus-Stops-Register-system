CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
LDFLAGS = -lpthread -lrt

SRC = Passengers.cpp Bus.cpp System.cpp Test.cpp
OBJ = $(SRC:.cpp=.o)

TARGET = bus_sim

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)
