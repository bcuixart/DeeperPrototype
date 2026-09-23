#ifndef PHYSICSMANAGER_HH
#define PHYSICSMANAGER_HH

#include <vector>
#include <algorithm>
#include <raylib.h>

class LevelManager;
class LevelObject;

class PhysicsManager {
public:
	PhysicsManager();
	~PhysicsManager();

	void Update(const float deltaTime, const LevelManager& levelManager);

    void RegisterObject(LevelObject* object);
    void UnregisterObject(LevelObject* object);

protected:
    
private:
    void ApplyGravity(const std::vector<LevelObject*>& objects, const float deltaTime);
    void MoveAndResolveCollisionsX(const std::vector<LevelObject*>& objects, const float deltaTime, bool blockedByRealSlopes);
    void MoveAndResolveCollisionsY(const std::vector<LevelObject*>& objects, const float deltaTime);
    bool CheckAndResolveCollisionX(LevelObject* stillObject, LevelObject* movingObject);
    bool CheckAndResolveCollisionXSlope(LevelObject* slopeObject, LevelObject* movingObject, bool blockedByRealSlopes);
    bool CheckAndResolveCollisionY(LevelObject* stillObject, LevelObject* movingObject);
    bool SweepYOnlyFromTop(const Rectangle& startBounds, float dy, LevelObject* stillObject, float& tOut);
    bool CheckAndResolveCollisionYSlope(LevelObject* slopeObject, LevelObject* movingObject);

    void RemoveObjectFromList(std::vector<LevelObject*>& list, LevelObject* obj);

    bool SweepY(const Rectangle& bounds, float dy, LevelObject* stillObject, float& tOut);
    bool SweepX(const Rectangle& bounds, float dx, LevelObject* stillObject, float& tOut);

    std::vector<LevelObject*> _solidObjects;
    std::vector<LevelObject*> _solidMaybeSlopedObjects;
    std::vector<LevelObject*> _semisolidTotalObjects;
    std::vector<LevelObject*> _semisolidPartialObjects;
    std::vector<LevelObject*> _entityObjects;
    std::vector<LevelObject*> _dynamicObjects;
    std::vector<LevelObject*> _forceFieldObjects;
    std::vector<LevelObject*> _triggerObjects;

    constexpr static float kGravity = 20.0f;
};

#endif