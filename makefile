CXX      = g++
CXXFLAGS = -std=c++17 -I. -Wall -Wextra -O2

# macOS Homebrew paths (works for both Intel and Apple Silicon)
BREW_PREFIX := $(shell brew --prefix 2>/dev/null || echo /usr/local)
CXXFLAGS += -I$(BREW_PREFIX)/include
LDFLAGS  = -L$(BREW_PREFIX)/lib

LIBS     = -lGL -lGLEW -lglfw -lassimp -lm

# If pkg-config is available, prefer its flags for glfw/glew/assimp
CXXFLAGS += $(shell pkg-config --cflags glew glfw3 assimp 2>/dev/null)
LIBS     += $(shell pkg-config --libs   glew glfw3 assimp 2>/dev/null)

SRCS     = main.cpp \
           shader.cpp \
           Shape.cpp \
           Transformations.cpp \
           Cylinder.cpp \
           Cuboid.cpp \
           Cone.cpp \
           TriangularPrism.cpp \
           TurfWall.cpp \
           PerimeterFenceMesh.cpp \
           GlbMesh.cpp \
           MapObjects.cpp \
           CourseLayout.cpp \
           Prototype.cpp \
           CourseObjects.cpp \
           FloorNode.cpp \
           RoadNode.cpp \
           WaterNode.cpp \
           RocksNode.cpp \
           BridgeNode.cpp \
           HutNode.cpp \
           Hole01Node.cpp \
           Hole02Node.cpp \
           Hole03Node.cpp \
           Hole04Node.cpp \
           Hole05Node.cpp \
           Hole06Node.cpp \
           Hole07Node.cpp \
           Hole08Node.cpp \
           Hole09Node.cpp \
           Hole10Node.cpp \
           Hole11Node.cpp \
           Hole12Node.cpp \
           Hole13Node.cpp \
           Hole14Node.cpp \
           Hole15Node.cpp \
           Hole16Node.cpp \
           Hole17Node.cpp \
           Hole18Node.cpp \
           WindmillNode.cpp \
           TreeNode.cpp \
           SceneRoot.cpp

OBJS     = $(SRCS:.cpp=.o)
TARGET   = minigolf

all: $(TARGET)

mac: CXX := clang++
mac: LIBS := -lGLEW -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -framework CoreFoundation
mac: LIBS += $(shell pkg-config --libs assimp 2>/dev/null || echo -lassimp)
mac: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean mac