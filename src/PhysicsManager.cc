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
	MoveAndResolveCollisionsX(_entityObjects, deltaTime, false);
	MoveAndResolveCollisionsX(_dynamicObjects, deltaTime, true);

	// Move objects on Y and resolve collisions
	MoveAndResolveCollisionsY(_entityObjects, deltaTime);
	MoveAndResolveCollisionsY(_dynamicObjects, deltaTime);
}

void PhysicsManager::ApplyGravity(const std::vector<LevelObject*>& objects, const float deltaTime)
{
	for (LevelObject* o : objects) o->AddVelocityY(kGravity * deltaTime);
}

void PhysicsManager::MoveAndResolveCollisionsX(const std::vector<LevelObject*>& objects, const float deltaTime, bool blockedByRealSlopes)
{
	for (LevelObject* o : objects)
	{
		// If not moving horizontally, skip
		float vel = o->GetVelocityX();
		if (vel == 0.0f) continue;

		// Apply horizontal movement
		o->ApplyVelocityX(deltaTime);

		// Check for collisions
		Rectangle bounds = o->GetBounds();
		for (LevelObject* solid : _solidObjects) 
		{
			if (CheckAndResolveCollisionXGround(solid, o, blockedByRealSlopes)) bounds = o->GetBounds();
		}

		for (LevelObject* solidMaybeSloped : _solidMaybeSlopedObjects) 
		{
			if (CheckAndResolveCollisionXSlope(solidMaybeSloped, o, blockedByRealSlopes)) bounds = o->GetBounds();
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

bool PhysicsManager::CheckAndResolveCollisionXGround(LevelObject* stillObject, LevelObject* movingObject, bool blockedByRealSlopes)
{
	if (!blockedByRealSlopes && IsRampBacking(stillObject))
		return false;

	return CheckAndResolveCollisionX(stillObject, movingObject);
}

bool PhysicsManager::CheckAndResolveCollisionXSlope(LevelObject* slopeObject, LevelObject* movingObject, bool blockedByRealSlopes)
{
    if (slopeObject->HasActualSlopedHitbox())
    {
        if (!blockedByRealSlopes) return false;

        Rectangle bounds = movingObject->GetBounds();
        Rectangle slopeBounds = slopeObject->GetBounds();

        if (bounds.y + bounds.height < slopeBounds.y || bounds.y > slopeBounds.y + slopeBounds.height) return false;
        if (bounds.x + bounds.width < slopeBounds.x || bounds.x > slopeBounds.x + slopeBounds.width) return false;

        float topY    = Clamp(bounds.y,                slopeBounds.y, slopeBounds.y + slopeBounds.height);
        float bottomY = Clamp(bounds.y + bounds.height, slopeBounds.y, slopeBounds.y + slopeBounds.height);

        Vector2 sampleTop    = slopeObject->SampleLeftRightAtY(topY);
        Vector2 sampleBottom = slopeObject->SampleLeftRightAtY(bottomY);

        const float rightEdge = bounds.x + bounds.width;
        const float leftEdge  = bounds.x;

        const bool rightInTop    = rightEdge >= sampleTop.x    && rightEdge <= sampleTop.y;
        const bool rightInBottom = rightEdge >= sampleBottom.x && rightEdge <= sampleBottom.y;

        if (rightInTop || rightInBottom)
        {
            float leftBoundary = 0.0f;
            if (rightInTop)    leftBoundary = sampleTop.x;
            if (rightInBottom) leftBoundary = rightInTop ? fminf(leftBoundary, sampleBottom.x) : sampleBottom.x;

            movingObject->SetBoundsX(leftBoundary - bounds.width);
            movingObject->SetVelocityX(0.0f);
            return true;
        }

        const bool leftInTop    = leftEdge >= sampleTop.x    && leftEdge <= sampleTop.y;
        const bool leftInBottom = leftEdge >= sampleBottom.x && leftEdge <= sampleBottom.y;

        if (leftInTop || leftInBottom)
        {
            float rightBoundary = 0.0f;
            if (leftInTop)    rightBoundary = sampleTop.y;
            if (leftInBottom) rightBoundary = leftInTop ? fmaxf(rightBoundary, sampleBottom.y) : sampleBottom.y;

            movingObject->SetBoundsX(rightBoundary);
            movingObject->SetVelocityX(0.0f);
            return true;
        }

        return false;
    }

    // Slope fals (pla): es comporta com un Ground normal. Només deixa passar
    // de llarg els entities si fa de replà net vora una rampa de veritat —
    // si no aporta res a cap rampa, bloqueja igual que qualsevol altre sòlid.
    if (!blockedByRealSlopes && IsRampBacking(slopeObject))
        return false;

    return CheckAndResolveCollisionX(slopeObject, movingObject);
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
			if (CheckAndResolveCollisionYGround(solid, o)) bounds = o->GetBounds();
		}

		for (LevelObject* solidMaybeSloped : _solidMaybeSlopedObjects) 
		{
			if (CheckAndResolveCollisionYSlope(solidMaybeSloped, o)) bounds = o->GetBounds();
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
	if (movingObject->GetVelocityY() == 0.0f) return false;

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

bool PhysicsManager::CheckAndResolveCollisionYGround(LevelObject* stillObject, LevelObject* movingObject)
{
	// Igual que en X: un Ground que toca una rampa real és, de fet, la seva
	// continuació plana. Es resol amb el mateix mètode que un Slope (mostreig
	// continu de top/bottom), no amb el bloqueig ple de tile, perquè la
	// transició sigui tan suau com la de la pròpia rampa.
	if (IsRampBacking(stillObject))
		return CheckAndResolveCollisionYSlope(stillObject, movingObject);

	return CheckAndResolveCollisionY(stillObject, movingObject);
}

bool PhysicsManager::CheckAndResolveCollisionYOnlyFromTop(LevelObject* stillObject, LevelObject* movingObject)
{
	// Check collision (Only from top)
	if (movingObject->GetVelocityY() <= 0.0f) return false;

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

bool PhysicsManager::CheckAndResolveCollisionYSlope(LevelObject* slopeObject, LevelObject* movingObject)
{
    Rectangle bounds = movingObject->GetBounds();
    Rectangle slopeBounds = slopeObject->GetBounds();

    float centerX = bounds.x + bounds.width * 0.5f;
    if (centerX < slopeBounds.x || centerX > slopeBounds.x + slopeBounds.width) return false;
    if (bounds.y + bounds.height < slopeBounds.y || bounds.y > slopeBounds.y + slopeBounds.height) return false;

    Vector2 sample = slopeObject->SampleTopBottomAtX(centerX);  // x = top, y = bottom
    const float top = sample.x;
    const float bottom = sample.y;

    const float feetY = bounds.y + bounds.height;
    const float headY = bounds.y;

    const bool feetInSlice = feetY >= top && feetY <= bottom;
    const bool headInSlice = headY >= top && headY <= bottom;
    const bool swallowed   = top >= headY && bottom <= feetY;

    bool isFloor;
    if (swallowed)
    {
        isFloor = (feetY - top) <= (bottom - headY);
    }
    else if (feetInSlice) isFloor = true;
    else if (headInSlice) isFloor = false;
    else return false;

    if (isFloor)
    {
        movingObject->SetBoundsY(top - bounds.height);
        movingObject->SetIsGrounded(true);
    }
    else
    {
        movingObject->SetBoundsY(bottom);
    }

    movingObject->SetVelocityY(0.0f);
    return true;
}

bool PhysicsManager::AreTilesHorizontallyAdjacent(LevelObject* a, LevelObject* b) const
{
	constexpr float kAdjacencyEpsilon = 1.0f;

	Rectangle boundsA = a->GetBounds();
	Rectangle boundsB = b->GetBounds();

	return (fabsf(boundsA.x - (boundsB.x + boundsB.width)) < kAdjacencyEpsilon ||
	        fabsf((boundsA.x + boundsA.width) - boundsB.x) < kAdjacencyEpsilon) &&
	       boundsA.y < boundsB.y + boundsB.height &&
	       boundsA.y + boundsA.height > boundsB.y;
}

bool PhysicsManager::HasWallDirectlyAbove(LevelObject* tile) const
{
	// Només compta Ground com a "mur a sobre". Un Slope (real o fals) a sobre
	// mai compta — és la pròpia estructura de la rampa continuant cap amunt
	// (el següent esglaó d'una rampa prima en forma d'escala, per exemple),
	// no un obstacle aliè.
	constexpr float kAdjacencyEpsilon = 1.0f;
	Rectangle bounds = tile->GetBounds();

	for (LevelObject* solid : _solidObjects)
	{
		if (solid == tile) continue;

		Rectangle otherBounds = solid->GetBounds();
		bool sameColumn = otherBounds.x < bounds.x + bounds.width && otherBounds.x + otherBounds.width > bounds.x;
		bool touchesTop = fabsf((otherBounds.y + otherBounds.height) - bounds.y) < kAdjacencyEpsilon;

		if (sameColumn && touchesTop) return true;
	}

	return false;
}

bool PhysicsManager::IsRampBacking(LevelObject* tile) const
{
	if (HasWallDirectlyAbove(tile)) return false;

	for (LevelObject* slope : _solidMaybeSlopedObjects)
	{
		if (!slope->HasActualSlopedHitbox()) continue;
		if (AreTilesHorizontallyAdjacent(tile, slope)) return true;
	}

	return false;
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