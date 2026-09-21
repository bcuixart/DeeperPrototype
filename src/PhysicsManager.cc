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
	// Reset grounded state for all objects
	for (LevelObject* o : _entityObjects) o->SetIsGrounded(false);
	for (LevelObject* o : _dynamicObjects) o->SetIsGrounded(false);

	// Apply gravity
	ApplyGravity(_entityObjects, deltaTime);
	ApplyGravity(_dynamicObjects, deltaTime);

	// Apply force fields


	// Move objects on X and resolve collisions
	MoveAndResolveCollisionsX(_entityObjects, deltaTime);
	MoveAndResolveCollisionsX(_dynamicObjects, deltaTime);

	// Move objects on Y and resolve collisions
	MoveAndResolveCollisionsY(_entityObjects, deltaTime);
	MoveAndResolveCollisionsY(_dynamicObjects, deltaTime);
}

void PhysicsManager::ApplyGravity(const std::vector<LevelObject*>& objects, const float deltaTime)
{
	for (LevelObject* o : objects) o->AddVelocityY(kGravity * deltaTime);
}

void PhysicsManager::MoveAndResolveCollisionsX(const std::vector<LevelObject*>& objects, const float deltaTime)
{
	for (LevelObject* o : objects)
	{
		// If not moving horizontally, skip
		float vel = o->GetVelocityX();
		if (vel == 0.0f) continue;

		o->ApplyVelocityX(deltaTime);

		// Check for collisions
		Rectangle bounds = o->GetBounds();
		for (LevelObject* solid : _solidObjects) 
		{
			if (CheckAndResolveCollisionX(solid, o)) bounds = o->GetBounds();
		}
	}
}

bool PhysicsManager::CheckAndResolveCollisionX(LevelObject* stillObject, LevelObject* movingObject)
{
	// Check collision
	Rectangle bounds = movingObject->GetBounds();
	Rectangle overlap = GetCollisionRec(bounds, stillObject->GetBounds());
	if (overlap.width <= 0.0f || overlap.height <= 0.0f) return false;

	// Resolve collision
	if (movingObject->GetVelocityX() > 0.0f) movingObject->SetBoundsX(bounds.x - overlap.width);
	else movingObject->SetBoundsX(bounds.x + overlap.width);

	movingObject->SetVelocityX(0.0f);
	return true;
}

void PhysicsManager::MoveAndResolveCollisionsY(const std::vector<LevelObject*>& objects, const float deltaTime)
{
	for (LevelObject* o : objects)
	{
		// If not moving vertically, skip
		float vel = o->GetVelocityY();
		if (vel == 0.0f) continue;

		o->ApplyVelocityY(deltaTime);

		// Check for collisions
		Rectangle bounds = o->GetBounds();
		for (LevelObject* solid : _solidObjects) 
		{
			if (CheckAndResolveCollisionY(solid, o)) bounds = o->GetBounds();
		}

		for (LevelObject* semisolidTotal : _semisolidTotalObjects) 
		{
			if (CheckAndResolveCollisionYOnlyFromTop(semisolidTotal, o)) bounds = o->GetBounds();
		}

		for (LevelObject* semisolidPartial : _semisolidPartialObjects) 
		{
			if (CheckAndResolveCollisionYOnlyFromTop(semisolidPartial, o)) bounds = o->GetBounds();
		}
	}
}

bool PhysicsManager::CheckAndResolveCollisionY(LevelObject* stillObject, LevelObject* movingObject)
{
	// Check collision
	Rectangle bounds = movingObject->GetBounds();
	Rectangle overlap = GetCollisionRec(bounds, stillObject->GetBounds());
	if (overlap.width <= 0.0f || overlap.height <= 0.0f) return false;

	// Resolve collision
	if (movingObject->GetVelocityY() > 0.0f) { movingObject->SetBoundsY(bounds.y - overlap.height); movingObject->SetIsGrounded(true); }
	else movingObject->SetBoundsY(bounds.y + overlap.height);

	movingObject->SetVelocityY(0.0f);
	return true;
}

bool PhysicsManager::CheckAndResolveCollisionYOnlyFromTop(LevelObject* stillObject, LevelObject* movingObject)
{
	// Check collision (Only from top)
	if (movingObject->GetVelocityY() < 0.0f) return false;

	Rectangle bounds = movingObject->GetBounds();
	Rectangle overlap = GetCollisionRec(bounds, stillObject->GetBounds());
	if (overlap.width <= 0.0f || overlap.height <= 0.0f) return false;

	float prevBottom = bounds.y + bounds.height - overlap.height;
	if (prevBottom > stillObject->GetBounds().y + 0.001f) return false;

	// Resolve collision
	movingObject->SetBoundsY(bounds.y - overlap.height);
	movingObject->SetIsGrounded(true);
	movingObject->SetVelocityY(0.0f);

	return true;
}

void PhysicsManager::RegisterObject(LevelObject* object)
{
	if (!object) return;

	HitboxType hitboxType = object->GetHitboxType();
	switch (hitboxType) 
	{
		case HitboxType::Solid:				_solidObjects.push_back(object); break;
		case HitboxType::SolidMaybeSloped:	_solidMaybeSlopedObjects.push_back(object); break;
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
		case HitboxType::SolidMaybeSloped:	RemoveObjectFromList(_solidMaybeSlopedObjects, object); break;
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