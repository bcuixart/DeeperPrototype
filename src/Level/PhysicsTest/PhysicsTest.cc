#include "Level/Level.hh"
#include "Level/LevelObjects/Test/LevelObjectTest.hh"
#include "Level/LevelObjects/Test/LevelObjectTestEntity.hh"
#include "Level/LevelObjects/Test/LevelObjectTestDynamic.hh"

#define SUCCESS 0
#define ERROR_LOAD_LEVEL -1
#define ERROR_RUN_TEST -2

struct PhysicsTest { std::string levelFileName; bool isTestObjEntity; Rectangle testObjBounds; Vector2 testObjVel; int ticks; };

int RunTest(const PhysicsTest& test)
{
    Level level(nullptr);
    if (!level.BuildLevelFromFile(test.levelFileName)) return ERROR_LOAD_LEVEL;

    //level.SpawnTestObject(test.tipus, test.testObjPos, test.testObjVel);
    level.SpawnTestObject(test.testObjBounds, test.testObjVel, test.isTestObjEntity);
    for (int i = 0; i < test.ticks; i++)
    {
        level.Update(1.0f / 60.0f);
    }

    return SUCCESS;
}

int main()
{
    PhysicsTest test;
    test.levelFileName = "tile_test";
    test.isTestObjEntity = true;
    test.testObjBounds = { 0.0f, 0.0f, 1.0f, 1.0f };
    test.testObjVel = { 0.0f, 0.0f };
    test.ticks = 60;

    int testResult = RunTest(test);
    if (testResult == SUCCESS) printf("PASSED\n");
    else if (testResult == ERROR_LOAD_LEVEL) printf("Error: Failed to load level\n");
    else if (testResult == ERROR_RUN_TEST) printf("FAILED\n");
}