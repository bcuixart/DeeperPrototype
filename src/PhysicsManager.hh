#ifndef PHYSICSMANAGER_HH
#define PHYSICSMANAGER_HH

#include <vector>
#include <algorithm>

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
    void RemoveObjectFromList(std::vector<LevelObject*>& list, LevelObject* obj);

    std::vector<LevelObject*> _solidObjects;
    std::vector<LevelObject*> _semisolidTotalObjects;
    std::vector<LevelObject*> _semisolidPartialObjects;
    std::vector<LevelObject*> _entityObjects;
    std::vector<LevelObject*> _dynamicObjects;
    std::vector<LevelObject*> _forceFieldObjects;
    std::vector<LevelObject*> _triggerObjects;
};

#endif