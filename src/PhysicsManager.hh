#ifndef PHYSICSMANAGER_HH
#define PHYSICSMANAGER_HH

#include <vector>
#include <algorithm>
#include <raylib.h>

class LevelManager;
class LevelObject;

class PhysicsManager {
public:
	PhysicsManager(LevelManager& levelManager);
	~PhysicsManager();

	void Update(const float deltaTime);

    void RegisterObject(LevelObject* object);
    void UnregisterObject(LevelObject* object);

protected:
    
private:
    void ApplyGravity(const std::vector<LevelObject*>& objects, const float deltaTime);

    void MoveAndResolveCollisionsX(const std::vector<LevelObject*>& objects, const float deltaTime, bool afectedByPartialSemisolids, bool blockedByRealSlopes);
    bool SweepX(const Rectangle& bounds, float dx, LevelObject* stillObject, float& tOut);
    bool SweepXSlope(const Rectangle& bounds, float dx, LevelObject* slopeObject, float& tOut);
    bool CheckAndResolveCollisionXSlope(LevelObject* slopeObject, LevelObject* movingObject, bool blockedByRealSlopes, float prevVelocityX);

    void MoveAndResolveCollisionsY(const std::vector<LevelObject*>& objects, const float deltaTime, bool checkSemisolidPartialBottom, bool affectedByEntitiesTop);
    bool SweepY(const Rectangle& bounds, float dy, LevelObject* stillObject, float& tOut);
    bool SweepYSlope(const Rectangle& bounds, float dy, LevelObject* slopeObject, float& tOut);
    bool SweepYOnlyFromTop(const Rectangle& startBounds, float dy, LevelObject* stillObject, float& tOut);
    bool CheckAndResolveCollisionYSlope(LevelObject* slopeObject, LevelObject* movingObject, float prevVelocityY);

    void CheckOverlaps(const std::vector<LevelObject*>& listA, const std::vector<LevelObject*>& listB);
    void CheckOverlapsSelf(const std::vector<LevelObject*>& list);

    void RemoveObjectFromList(std::vector<LevelObject*>& list, LevelObject* obj);

    std::vector<LevelObject*> _solidObjects;
    std::vector<LevelObject*> _solidMaybeSlopedObjects;
    std::vector<LevelObject*> _solidBreakableObjects;
    std::vector<LevelObject*> _semisolidTotalObjects;
    std::vector<LevelObject*> _semisolidPartialObjects;
    std::vector<LevelObject*> _entityObjects;
    std::vector<LevelObject*> _dynamicObjects;
    std::vector<LevelObject*> _forceFieldObjects;
    std::vector<LevelObject*> _triggerObjects;

    LevelManager* _levelManager{nullptr};

    constexpr static float kGravity = 20.0f;
};

#endif