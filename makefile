CXX      = g++
CXXFLAGS = -std=c++17 -I. -Wall -Wextra -O2

LIBS     = -lGL -lGLEW -lglfw -lm

SRCS     = main.cpp \
           shader.cpp \
           Shape.cpp \
           Transformations.cpp \
           Cylinder.cpp \
           Cuboid.cpp \
           Cone.cpp \
           TriangularPrism.cpp \
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

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
