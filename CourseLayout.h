#ifndef COURSELAYOUT_H
#define COURSELAYOUT_H

#include <GL/glew.h>
#include <vector>

struct FlatPoly {
    GLuint vao = 0, vbo = 0;
    int triCount = 0;
    void build(const float pts[][2], int n,
               float r, float g, float b, float y = 0.0f);
    void draw()    const;
    void cleanup();
};

struct Disc {
    GLuint vao = 0, vbo = 0;
    int triCount = 0;
    void build(float cx, float cz, float radius,
               float r, float g, float b,
               float y = 0.0f, int segs = 12);
    void draw()    const;
    void cleanup();
};

class CourseLayout {
public:
    CourseLayout();
    ~CourseLayout();
    void build();
    void draw()    const;   // draws everything (legacy / fallback)
    void cleanup();

    // ── Per-category draw methods ────────────────────────────
    //  Called by the individual SceneNode builders so each
    //  object type can be its own node in the scene graph.
    void drawFloor()      const { m_floor.draw(); }
    void drawRoad()       const { m_road.draw(); }
    void drawWater()      const { for(int i=0;i<3;++i) m_dams[i].draw(); }
    void drawRocks()      const {
        for(int i=0;i<10;++i) m_rockbedPoly[i].draw();
        for(int i=0;i<m_rockDiscCount;++i) m_rockDiscs[i].draw();
    }
    void drawGreenbed(int hole) const {   // hole = 0..17
        m_greenbedSand  [hole].draw();
        m_greenbedRing  [hole].draw();
        m_greenbedCentre[hole].draw();
    }
    void drawGreen(int hole)   const { m_holes[hole].draw(); }
    void drawBridges()         const { m_bridge[0].draw(); m_bridge[1].draw(); }
    void drawHut()             const { m_hut.draw(); }
    void drawFlag(int hole)    const { m_flags[hole].draw(); }  // flat disc flag

private:
    FlatPoly m_floor;               // sand/flesh base
    FlatPoly m_road;                // grey road
    FlatPoly m_dams[3];             // dam 1/2/3 water bodies
    FlatPoly m_rockbedPoly[10];     // filled rock bed areas
    Disc     m_rockDiscs[200];      // individual rock discs
    int      m_rockDiscCount = 0;
    FlatPoly m_greenbedSand[18];
    FlatPoly m_greenbedRing[18];
    FlatPoly m_greenbedCentre[18];
    FlatPoly m_holes[18];
    FlatPoly m_bridge[2];
    FlatPoly m_hut;
    Disc     m_flags[18];
    bool     m_built = false;
};

#endif // COURSELAYOUT_H