CXX = g++
CXXFLAGS = -std=c++17
LDFLAGS = -L/opt/homebrew/lib -lcurl -pthread

TARGET = fuzzer
SRC = $(wildcard *.cpp)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)
