# Compiler settings
CXX = g++
CXXFLAGS = -Wall -O2 -fPIC -DUSE_GLES -I./luna-mbx -I./mbx_src/Project/Source $(shell pkg-config --cflags sdl2 SDL2_mixer)
LDFLAGS = $(shell pkg-config --libs sdl2 SDL2_mixer) -lGLESv1_CM

# Source files
LUNA_SOURCES = $(wildcard luna-mbx/*.cc)
GAME_SOURCES = $(wildcard mbx_src/Project/Source/*.cpp)

# Object files
LUNA_OBJECTS = $(LUNA_SOURCES:.cc=.o)
GAME_OBJECTS = $(GAME_SOURCES:.cpp=.o)

# Target executable
TARGET = magicalbroom

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(LUNA_OBJECTS) $(GAME_OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Compilation rules
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean
clean:
	rm -f $(LUNA_OBJECTS) $(GAME_OBJECTS) $(TARGET)

.PHONY: all clean