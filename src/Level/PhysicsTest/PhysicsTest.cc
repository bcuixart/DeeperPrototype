#include <iostream>

#include "Level/Level.hh"
#include "Level/LevelObjects/Test/LevelObjectTest.hh"
#include "Level/LevelObjects/Test/LevelObjectTestEntity.hh"
#include "Level/LevelObjects/Test/LevelObjectTestDynamic.hh"

#define SUCCESS 0
#define ERROR_LOAD_LEVEL -1
#define ERROR_RUN_TEST -2

struct PhysicsTest { std::string levelFileName; bool isTestObjEntity; Rectangle testObjBounds; Vector2 testObjVel; int ticks; };
struct Result { float finalFeet; float minHead; bool grounded; };

Result Run(Level& level, const Rectangle& b, Vector2 vel, bool isEntity, int ticks)
{
    LevelObjectTest* o = level.SpawnTestObject(b, vel, isEntity);
    Result r{0.0f, 1e9f, false};
    for (int i = 0; i < ticks; ++i)
    {
        level.Update(1.0f / 60.0f);
        r.minHead = std::min(r.minHead, o->GetBounds().y);
    }
    const Rectangle f = o->GetBounds();
    r.finalFeet = f.y + f.height;
    r.grounded = o->GetIsGrounded();
    return r;
}

int RunTest(const PhysicsTest& test, int& lastTickExecuted)
{
    const float bottom = 6.0f;          // substitueix pel bottom real del parcial
    const float size = 0.4f;
    const float cx = 25.5f;

    struct Cas { float capOffset; float vy; };
    const Cas casos[] = {
        {0.000f, -5}, {0.002f, -5}, {0.005f, -5}, {0.009f, -5},   // casos t = 0
        {0.3f, -3}, {0.3f, -10}, {0.3f, -30}                       // casos generals
    };

    for (const Cas& c : casos)
    {
        Level level(nullptr); level.BuildLevelFromFile(test.levelFileName);
        Result r = Run(level, {cx - size * 0.5f, bottom + c.capOffset, size, size}, {0, c.vy}, false, 120);
        if (r.minHead < bottom - 0.02f)
            printf("FALLA capOffset=%.3f vy=%.0f minCap=%.3f\n", c.capOffset, c.vy, r.minHead);
    }

    return 0;
}

int main()
{
    PhysicsTest test;
    test.levelFileName = "physics_test/physics_test_01";
    test.isTestObjEntity = false;
    test.testObjBounds = { 4.9f, 0.1f, 1.0f, 1.0f };
    test.testObjVel = { 0.0f, 0.0f };
    test.ticks = 60;

    int lastTickExecuted = 0;
    int testResult = RunTest(test, lastTickExecuted);
    (void)testResult; (void)lastTickExecuted;
}