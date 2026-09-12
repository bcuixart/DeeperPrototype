#include "PhysicsManager.hh"
#include "LevelManager.hh"

PhysicsManager::PhysicsManager()
{

}

PhysicsManager::~PhysicsManager()
{
}

void PhysicsManager::Update(const float deltaTime, const LevelManager& levelManager)
{

}

void PhysicsManager::RegisterObject(LevelObject* object)
{
	if (!object) return;

	HitboxType hitboxType = object->GetHitboxType();
	switch (hitboxType) 
	{
		case HitboxType::Solid:				_solidObjects.push_back(object); break;
		case HitboxType::SemisolidTotal:	_semisolidTotalObjects.push_back(object); break;
		case HitboxType::SemisolidPartial:	_semisolidPartialObjects.push_back(object); break;
		case HitboxType::Entity:			_entityObjects.push_back(object); break;
		case HitboxType::Dynamic:			_dynamicObjects.push_back(object); break;
		case HitboxType::ForceField:		_forceFieldObjects.push_back(object); break;
		case HitboxType::Trigger:			_triggerObjects.push_back(object); break;
		default: break;
	}
}

void PhysicsManager::UnregisterObject(LevelObject* object)
{
	if (!object) return;

	HitboxType hitboxType = object->GetHitboxType();
	switch (hitboxType) 
	{
		case HitboxType::Solid:				RemoveObjectFromList(_solidObjects, object); break;
		case HitboxType::SemisolidTotal:	RemoveObjectFromList(_semisolidTotalObjects, object); break;
		case HitboxType::SemisolidPartial:	RemoveObjectFromList(_semisolidPartialObjects, object); break;
		case HitboxType::Entity:			RemoveObjectFromList(_entityObjects, object); break;
		case HitboxType::Dynamic:			RemoveObjectFromList(_dynamicObjects, object); break;
		case HitboxType::ForceField:		RemoveObjectFromList(_forceFieldObjects, object); break;
		case HitboxType::Trigger:			RemoveObjectFromList(_triggerObjects, object); break;
		default: break;
	}
}

void PhysicsManager::RemoveObjectFromList(std::vector<LevelObject*>& list, LevelObject* obj)
{
	list.erase(std::remove(list.begin(), list.end(), obj), list.end());
}