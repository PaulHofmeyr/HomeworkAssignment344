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

// One VAO that holds many polygons/discs concatenated — one glDrawArrays call.
struct BatchedFlat {
    GLuint vao = 0, vbo = 0;
    int vertexCount = 0;
    void upload(const std::vector<float>& data);
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
    void draw()    const;
    void cleanup();

    // ── Per-category draw methods ────────────────────────────
    void drawFloor()        const { m_floor.draw(); }
    void drawRoad()         const { m_road.draw(); }
    void drawWater()        const { for(int i=0;i<3;++i) m_dams[i].draw(); }
    void drawRocks()        const {
        for(int i=0;i<10;++i) m_rockbedPoly[i].draw();
        m_batchRockDiscs.draw();   // all rock discs in one call
    }
    // Per-hole draw methods — used by individual HoleXXNode files
    void drawGreenbed(int i) const { if(i>=0&&i<18) m_greenbed[i].draw(); }
    void drawGreen   (int i) const { if(i>=0&&i<18) m_green[i].draw();    }
    void drawFlag    (int i) const { if(i>=0&&i<18) m_flag[i].draw();     }

    // Batched convenience — draws all 18 at once (used by SceneRoot if needed)
    void drawAllGreenbeds() const { for(int i=0;i<18;++i) m_greenbed[i].draw(); }
    void drawAllGreens()    const { for(int i=0;i<18;++i) m_green[i].draw();    }
    void drawAllFlags()     const { for(int i=0;i<18;++i) m_flag[i].draw();     }

    void drawBridges()      const { m_bridge[0].draw(); m_bridge[1].draw(); }
    void drawHut()          const { m_hut.draw(); }

private:
    FlatPoly    m_floor;
    FlatPoly    m_road;
    FlatPoly    m_dams[3];
    FlatPoly    m_rockbedPoly[10];
    BatchedFlat m_batchRockDiscs;
    BatchedFlat m_greenbed[18];   // one per hole: sand + ring + centre
    BatchedFlat m_green[18];      // one per hole: putting green
    BatchedFlat m_flag[18];       // one per hole: flag disc
    FlatPoly    m_bridge[2];
    FlatPoly    m_hut;
    bool        m_built = false;
};

#endif // COURSELAYOUT_H
