#include "Scene.h"
#include "Bollard.h"
#include "AppState.h"

// Colours
#define COL_GRASS 0.25f, 0.62f, 0.25f    // green
#define COL_WALL 0.35f, 0.28f, 0.20f     // dark brown
#define COL_STARTMAT 0.65f, 0.10f, 0.10f // red/maroon
#define COL_HOLE 0.08f, 0.08f, 0.08f     // near black
#define COL_RAMP 0.55f, 0.38f, 0.18f     // sandy ramp colour

// Bollard: dark grey body with a slightly lighter domed cap
#define COL_BOLLARD_BODY 0.18f, 0.18f, 0.20f
#define COL_BOLLARD_CAP  0.28f, 0.28f, 0.30f

#define COL_STONE 0.78f, 0.70f, 0.55f    // sandstone
#define COL_DARK_CAP 0.22f, 0.22f, 0.22f // dark grey
#define COL_ROOF 0.15f, 0.15f, 0.15f     // near black
#define COL_AXLE 0.60f, 0.60f, 0.65f     // metallic silver
#define COL_BLADE 0.92f, 0.92f, 0.88f    // off-white

// Round tree
#define COL_TRUNK_ROUND 0.45f, 0.28f, 0.12f
#define COL_TOP_ROUND 0.18f, 0.52f, 0.18f

// Triangular tree
#define COL_TRUNK_PRISM 0.30f, 0.18f, 0.08f
#define COL_TOP_PRISM 0.12f, 0.38f, 0.12f

// Shape pointers

// Course
static Cuboid *grassFloor = nullptr;
static Cuboid *wallNorth = nullptr;
static Cuboid *wallSouth = nullptr;
static Cuboid *wallEast = nullptr;
static Cuboid *wallWest = nullptr;
static Cuboid *startMat = nullptr;
static Cylinder *golfHole = nullptr;

// Ramps
static TriangularPrism *rampNorth = nullptr;
static TriangularPrism *rampSouth = nullptr;

// Round trees
static Cylinder *roundTrunk[4];
static Cone *roundTop[4];

// Prism trees
static Cylinder *prismTrunk[4];
static TriangularPrism *prismTop[4];

// Windmill legs
static Cuboid *leg0 = nullptr;
static Cuboid *leg1 = nullptr;
static Cuboid *leg2 = nullptr;
static Cuboid *leg3 = nullptr;
static Cuboid *leg4 = nullptr;
static Cuboid *leg5 = nullptr;

// Windmill tower + roof
static Cylinder *tower = nullptr;
static Cylinder *darkCap = nullptr;
static Cone *roof = nullptr;

// Rotor
static Cylinder *axle = nullptr;
static Cuboid *blade0 = nullptr;
static Cuboid *blade1 = nullptr;
static Cuboid *blade2 = nullptr;
static Cuboid *blade3 = nullptr;

// ── Bollards ──────────────────────────────────────────────────────────────────
// One prototype built once; each placed instance shares its GPU buffers.
// To add more bollards: push another Bollard* into bollardInstances and
// add a Matrix<4,4> in bollardTransforms below.
static Bollard *bollardProto = nullptr;   // prototype — owns GPU data
static const int NUM_BOLLARD_INSTANCES = 1;
static Bollard  *bollardInstances[NUM_BOLLARD_INSTANCES];
static Matrix<4,4> bollardTransforms[NUM_BOLLARD_INSTANCES];

void initScene()
{
    // Course
    grassFloor = new Cuboid(0.0f, -0.05f, 0.0f, 1.8f, 0.05f, 2.8f, COL_GRASS);

    wallNorth = new Cuboid(0.0f, 0.12f, 2.8f, 1.8f, 0.17f, 0.05f, COL_WALL);
    wallSouth = new Cuboid(0.0f, 0.12f, -2.8f, 1.8f, 0.17f, 0.05f, COL_WALL);
    wallEast = new Cuboid(1.8f, 0.12f, 0.0f, 0.05f, 0.17f, 2.8f, COL_WALL);
    wallWest = new Cuboid(-1.8f, 0.12f, 0.0f, 0.05f, 0.17f, 2.8f, COL_WALL);

    // Starting mat
    startMat = new Cuboid(0.0f, 0.01f, 2.0f, 0.30f, 0.01f, 0.30f, COL_STARTMAT);

    // Golf hole
    golfHole = new Cylinder(0.0f, 0.01f, -2.0f, 0.08f, 0.02f, 16, COL_HOLE);

    // Ramps
    rampNorth = new TriangularPrism(0.0f, 0.05f, 2.65f,
                                    0.5f, 0.08f, -0.15f,
                                    COL_RAMP);
    rampSouth = new TriangularPrism(0.0f, 0.05f, -2.65f,
                                    0.5f, 0.08f, 0.15f,
                                    COL_RAMP);

    // Round Trees
    float roundX[4] = {-2.3f, 2.3f, -2.3f, 2.3f};
    float roundZ[4] = {-1.0f, 0.5f, 1.5f, -1.5f};

    for (int i = 0; i < 4; i++)
    {
        roundTrunk[i] = new Cylinder(roundX[i], 0.15f, roundZ[i],
                                     0.07f, 0.40f, 8,
                                     COL_TRUNK_ROUND);
        roundTop[i] = new Cone(roundX[i], 0.35f, roundZ[i],
                               0.22f, 0.55f, 8,
                               COL_TOP_ROUND);
    }

    // Prism Trees
    float prismX[4] = {-2.3f, 2.3f, 0.0f, 0.0f};
    float prismZ[4] = {0.0f, 2.0f, -3.2f, 3.2f};

    for (int i = 0; i < 4; i++)
    {
        prismTrunk[i] = new Cylinder(prismX[i], 0.15f, prismZ[i],
                                     0.07f, 0.40f, 8,
                                     COL_TRUNK_PRISM);
        prismTop[i] = new TriangularPrism(prismX[i], 0.55f, prismZ[i],
                                          0.22f, 0.22f, 0.22f,
                                          COL_TOP_PRISM);
    }

    // Windmill legs
    float legR = 0.38f;
    float legHW = 0.07f;
    float legHH = 0.28f;
    float legHD = 0.07f;
    float legCY = legHH;

    leg0 = new Cuboid(legR * 1.00f, legCY, legR * 0.00f, legHW, legHH, legHD, COL_STONE);
    leg1 = new Cuboid(legR * 0.50f, legCY, legR * 0.87f, legHW, legHH, legHD, COL_STONE);
    leg2 = new Cuboid(-legR * 0.50f, legCY, legR * 0.87f, legHW, legHH, legHD, COL_STONE);
    leg3 = new Cuboid(-legR * 1.00f, legCY, legR * 0.00f, legHW, legHH, legHD, COL_STONE);
    leg4 = new Cuboid(-legR * 0.50f, legCY, -legR * 0.87f, legHW, legHH, legHD, COL_STONE);
    leg5 = new Cuboid(legR * 0.50f, legCY, -legR * 0.87f, legHW, legHH, legHD, COL_STONE);

    // Windmill tower and roof
    tower = new Cylinder(0.0f, 0.84f, 0.0f, 0.35f, 0.95f, 16, COL_STONE);
    darkCap = new Cylinder(0.0f, 1.40f, 0.0f, 0.37f, 0.18f, 16, COL_DARK_CAP);
    roof = new Cone(0.0f, 1.49f, 0.0f, 0.37f, 0.40f, 16, COL_ROOF);

    // Rotor
    axle = new Cylinder(0.0f, 1.48f, 0.43f, 0.06f, 0.03f, 12, COL_AXLE);

    // Blade hub
    float bCX = 0.0f, bCY = 1.48f, bCZ = 0.43f;
    float bLen = 0.35f, bW = 0.05f, bD = 0.015f;

    // Blades
    blade0 = new Cuboid(bCX, bCY + bLen / 2.0f, bCZ, bW, bLen, bD, COL_BLADE);
    blade1 = new Cuboid(bCX + bLen / 2.0f, bCY, bCZ, bLen, bW, bD, COL_BLADE);
    blade2 = new Cuboid(bCX, bCY - bLen / 2.0f, bCZ, bW, bLen, bD, COL_BLADE);
    blade3 = new Cuboid(bCX - bLen / 2.0f, bCY, bCZ, bLen, bW, bD, COL_BLADE);

    // Build buffers
    grassFloor->build();
    wallNorth->build();
    wallSouth->build();
    wallEast->build();
    wallWest->build();
    startMat->build();
    golfHole->build();
    rampNorth->build();
    rampSouth->build();

    for (int i = 0; i < 4; i++)
    {
        roundTrunk[i]->build();
        roundTop[i]->build();
        prismTrunk[i]->build();
        prismTop[i]->build();
    }

    leg0->build();
    leg1->build();
    leg2->build();
    leg3->build();
    leg4->build();
    leg5->build();

    tower->build();
    darkCap->build();
    roof->build();

    axle->build();
    blade0->build();
    blade1->build();
    blade2->build();
    blade3->build();

    // ── Bollards ──────────────────────────────────────────────────────────────
    // Build the prototype once at the origin; instances share its buffers.
    bollardProto = new Bollard(0.0f, 0.0f, 0.0f, COL_BOLLARD_BODY);
    bollardProto->build();

    // Instance 0: placed beside the starting mat on the east side
    bollardInstances[0] = new Bollard(0.0f, 0.0f, 0.0f, COL_BOLLARD_BODY);
    bollardInstances[0]->cloneBuffers(*bollardProto);
    bollardTransforms[0] = makeTranslation3D(-1.4f, 0.0f, 2.0f);
    // Add more instances here by increasing NUM_BOLLARD_INSTANCES above,
    // creating a new Bollard*, calling cloneBuffers, and setting its transform.
}

void drawScene(bool wireframe)
{
    if (wireframe)
    {
        grassFloor->drawWireframe();
        wallNorth->drawWireframe();
        wallSouth->drawWireframe();
        wallEast->drawWireframe();
        wallWest->drawWireframe();
        startMat->drawWireframe();
        golfHole->drawWireframe();
        rampNorth->drawWireframe();
        rampSouth->drawWireframe();

        for (int i = 0; i < 4; i++)
        {
            roundTrunk[i]->drawWireframe();
            roundTop[i]->drawWireframe();
            prismTrunk[i]->drawWireframe();
            prismTop[i]->drawWireframe();
        }

        leg0->drawWireframe();
        leg1->drawWireframe();
        leg2->drawWireframe();
        leg3->drawWireframe();
        leg4->drawWireframe();
        leg5->drawWireframe();

        tower->drawWireframe();
        darkCap->drawWireframe();
        roof->drawWireframe();
    }
    else
    {
        grassFloor->drawFilled();
        wallNorth->drawFilled();
        wallSouth->drawFilled();
        wallEast->drawFilled();
        wallWest->drawFilled();
        startMat->drawFilled();
        golfHole->drawFilled();
        rampNorth->drawFilled();
        rampSouth->drawFilled();

        for (int i = 0; i < 4; i++)
        {
            roundTrunk[i]->drawFilled();
            roundTop[i]->drawFilled();
            prismTrunk[i]->drawFilled();
            prismTop[i]->drawFilled();
        }

        leg0->drawFilled();
        leg1->drawFilled();
        leg2->drawFilled();
        leg3->drawFilled();
        leg4->drawFilled();
        leg5->drawFilled();

        tower->drawFilled();
        darkCap->drawFilled();
        roof->drawFilled();
    }

    // ── Bollard instances (always drawn; modelMatrix uploaded per instance) ───
    {
        AppState &app = AppState::get();
        GLuint prog = app.sceneShaderID;
        for (int i = 0; i < NUM_BOLLARD_INSTANCES; i++) {
            float flat[16];
            flattenMatrix4(bollardTransforms[i], flat);
            glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, flat);
            if (wireframe)
                bollardInstances[i]->drawWireframe();
            else
                bollardInstances[i]->drawFilled();
        }
        // Restore identity model matrix for subsequent callers
        float id[16]; flattenMatrix4(getIdentity4(), id);
        glUniformMatrix4fv(glGetUniformLocation(prog, "modelMatrix"), 1, GL_FALSE, id);
    }
}

void drawRotor(bool wireframe)
{
    if (wireframe)
    {
        axle->drawWireframe();
        blade0->drawWireframe();
        blade1->drawWireframe();
        blade2->drawWireframe();
        blade3->drawWireframe();
    }
    else
    {
        axle->drawFilled();
        blade0->drawFilled();
        blade1->drawFilled();
        blade2->drawFilled();
        blade3->drawFilled();
    }
}

void cleanupScene()
{
    delete grassFloor;
    delete wallNorth;
    delete wallSouth;
    delete wallEast;
    delete wallWest;
    delete startMat;
    delete golfHole;
    delete rampNorth;
    delete rampSouth;

    for (int i = 0; i < 4; i++)
    {
        delete roundTrunk[i];
        delete roundTop[i];
        delete prismTrunk[i];
        delete prismTop[i];
    }

    delete leg0;
    delete leg1;
    delete leg2;
    delete leg3;
    delete leg4;
    delete leg5;

    delete tower;
    delete darkCap;
    delete roof;

    delete axle;
    delete blade0;
    delete blade1;
    delete blade2;
    delete blade3;
}
// Rotor spinning transform
static Matrix<4, 4> g_rotorTransform = getIdentity4();

void setRotorTransform(const Matrix<4, 4> &m) { g_rotorTransform = m; }