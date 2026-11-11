#include "LevelObjectTest.hh"
#include "Level/Level.hh"

LevelObjectTest::LevelObjectTest(const Rectangle& bounds, Level* level)
    : _level(level)
{
    _bounds = bounds;   
}

void LevelObjectTest::Update(const float deltaTime) 
{

}

void LevelObjectTest::Render(const float deltaTime) const 
{

}
