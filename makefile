CXX      = g++
CXXFLAGS = -std=c++17 -I. -Wall -Wextra -O2

# macOS Homebrew paths (works for both Intel and Apple Silicon)
BREW_PREFIX := $(shell brew --prefix 2>/dev/null || echo /usr/local)
CXXFLAGS += -I$(BREW_PREFIX)/include
LDFLAGS  = -L$(BREW_PREFIX)/lib

LIBS     = -lGL -lGLEW -lglfw -lm

SRCS     = main.cpp \
           shader.cpp \
           Shape.cpp \
           Transformations.cpp \
           Cylinder.cpp \
           Cuboid.cpp \
           Cone.cpp \
           TriangularPrism.cpp \
           Bollard.cpp \
           Scene.cpp \
           CourseLayout.cpp \
           Prototype.cpp \
           CourseObjects.cpp \
           FloorNode.cpp \
           RoadNode.cpp \
           WaterNode.cpp \
           RocksNode.cpp \
           BridgeNode.cpp \
           HutNode.cpp \
           HoleNode.cpp \
           WindmillNode.cpp \
           TreeNode.cpp \
           SceneRoot.cpp

OBJS     = $(SRCS:.cpp=.o)
TARGET   = minigolf

all: $(TARGET)

mac: LIBS := -lGLEW -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreFoundation
mac: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean mac