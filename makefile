CXX      = g++
CXXFLAGS = -std=c++17 -I. -Wall -Wextra -O2

LIBS     = -lGL -lGLEW -lglfw -lm

# Note: Vector.cpp and Matrix.cpp are excluded — they are #included by their headers.
# shader.cpp is the original file from the project (unchanged).
SRCS     = main.cpp \
           shader.cpp \
           Shape.cpp \
           Transformations.cpp \
           Cylinder.cpp \
           Cuboid.cpp \
           Cone.cpp \
           TriangularPrism.cpp \
           Scene.cpp

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