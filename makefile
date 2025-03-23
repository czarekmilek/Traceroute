# Cezary Miłek 339746

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
TARGET = traceroute
SRC = main.cpp icmp_utils.cpp traceroute_utils.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o

distclean: clean
