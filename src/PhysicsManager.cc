#include "PhysicsManager.hh"
#include "LevelManager.hh"

PhysicsManager::PhysicsManager(LevelManager& levelManager) : _levelManager(&levelManager)
{

}

PhysicsManager::~PhysicsManager()
{
	
}

void PhysicsManager::Update(const float deltaTime)
{
	// Reset grounded state for all objects
	for (LevelObject* o : _entityObjects) o->SetIsGrounded(false);
	for (LevelObject* o : _dynamicObjects) o->SetIsGrounded(false);

	// Apply gravity
	ApplyGravity(_entityObjects, deltaTime);
	ApplyGravity(_dynamicObjects, deltaTime);

	// Apply force fields


	// Move objects on X and resolve collisions
	MoveAndResolveCollisionsX(_entityObjects, deltaTime, false, false);
	MoveAndResolveCollisionsX(_dynamicObjects, deltaTime, true, true);

	// Move objects on Y and resolve collisions
	MoveAndResolveCollisionsY(_entityObjects, deltaTime, false, true);
	MoveAndResolveCollisionsY(_dynamicObjects, deltaTime, true, false);

	// Check overlap and handle interactions between entities and dynamics
	CheckOverlapsSelf(_entityObjects);
	CheckOverlaps(_entityObjects, _dynamicObjects);
	CheckOverlapsSelf(_dynamicObjects);

	// Check overlap and handle interactions with triggers
	CheckOverlaps(_entityObjects, _triggerObjects);
	CheckOverlaps(_dynamicObjects, _triggerObjects);
}

void PhysicsManager::ApplyGravity(const std::vector<LevelObject*>& objects, const float deltaTime)
{
	for (LevelObject* o : objects) o->AddVelocityY(kGravity * deltaTime);
}

void PhysicsManager::MoveAndResolveCollisionsX(const std::vector<LevelObject*>& objects, const float deltaTime, bool affectedByPartialSemisolids, bool blockedByRealSlopes)
{
	for (LevelObject* o : objects)
	{
		float vel = o->GetVelocityX();
		if (vel == 0.0f) continue;

		Rectangle bounds = o->GetBounds();

		// We sweep against all non-sloped tiles to find the proportion of dx (tMin) of the first collision.
		const float dx = vel * deltaTime;
		float tMin = 1.0f;
		bool hitFound = false;
		LevelObject* hitObject = nullptr;

		for (LevelObject* solid : _solidObjects)
		{
			float t;
			if (SweepX(bounds, dx, solid, t) && t < tMin) { tMin = t; hitFound = true; hitObject = solid;}
		}

		for (LevelObject* slope : _solidMaybeSlopedObjects)
		{
			float t;
			if (slope->HasActualSlopedHitbox())
			{
				if (blockedByRealSlopes && SweepXSlope(bounds, dx, slope, t) && t < tMin) { tMin = t; hitFound = true; hitObject = slope; }
			}
			else
			{
				if (SweepX(bounds, dx, slope, t) && t < tMin) { tMin = t; hitFound = true; hitObject = slope; }
			}
		}

		for (LevelObject* breakable : _solidBreakableObjects)
		{
			if (breakable->GetBreakableIsBroken()) continue;

			float t;
			if (!SweepX(bounds, dx, breakable, t)) continue;

			if (o->GetVelocityMagnitude() >= breakable->GetBreakableBreakSpeed())
			{
				breakable->BreakableBreak(o);
				continue; // Ignore collision if it goes fast enough to break it
			}

			if (t < tMin) { tMin = t; hitFound = true; hitObject = breakable; }
		}

		if (affectedByPartialSemisolids)
		{
			for (LevelObject* semisolidPartial : _semisolidPartialObjects)
			{
				float t;
				if (SweepX(bounds, dx, semisolidPartial, t) && t < tMin) { tMin = t; hitFound = true; hitObject = semisolidPartial; }
			}
		}

		// Move the object to the new position based on tMin
		o->SetBoundsX(bounds.x + dx * tMin);
		if (hitFound)
		{
			o->SetVelocityX(0.0f);
			o->CollidedWithX(hitObject, vel);
			if (hitObject) hitObject->CollidedWithX(o, vel);
		}

		// After moving, we check for collisions with actually sloped tiles and resolve them if necessary.
		// This is done after the initial move to ensure that we handle slopes correctly, especially if the object is moving into a slope.
		bounds = o->GetBounds();
		for (LevelObject* solidMaybeSloped : _solidMaybeSlopedObjects)
		{
			if (!solidMaybeSloped->HasActualSlopedHitbox()) continue;
			if (CheckAndResolveCollisionXSlope(solidMaybeSloped, o, blockedByRealSlopes, vel)) bounds = o->GetBounds();
		}
	}
}

bool PhysicsManager::SweepX(const Rectangle& bounds, float dx, LevelObject* stillObject, float& tOut)
{
	if (dx == 0.0f) return false;

	// Check vertical overlap first
	Rectangle stillBounds = stillObject->GetBounds();
	if (bounds.y + bounds.height <= stillBounds.y || bounds.y >= stillBounds.y + stillBounds.height) return false;

	// Discard collision if it comes from a side that the still object does not have as solid
	const bool movingRight = dx > 0.0f;
	const uint8_t relevantSide = movingRight ? SOLID_SIDE_LEFT : SOLID_SIDE_RIGHT;
	if (!stillObject->GetSolidSideCollisionMask(relevantSide)) return false;

	// Find the edges that might collide and get their gap
	float leadingX, targetX;
	if (movingRight) { leadingX = bounds.x + bounds.width; targetX = stillBounds.x; }
	else              { leadingX = bounds.x;                targetX = stillBounds.x + stillBounds.width; }
	const float gap = movingRight ? (targetX - leadingX) : (leadingX - targetX);

	// kPenetrationEpsilon is a small value to account for floating-point inaccuracies and prevent objects from getting stuck when they are very close to each other
	constexpr float kPenetrationEpsilon = 0.01f;

	// If the gap is negative, it means we are already overlapping. If it's positive, we can calculate the time of collision
	float t;
	if (gap <= 0.0f)
	{
		if (gap < -kPenetrationEpsilon) return false;
		t = 0.0f;
	}
	else t = gap / fabsf(dx);

	// If t is not between 0 and 1 the objects will not collide this frame
	if (t < 0.0f || t > 1.0f) return false;

	tOut = t;
	return true;
}

bool PhysicsManager::SweepXSlope(const Rectangle& bounds, float dx, LevelObject* slopeObject, float& tOut)
{
	if (dx == 0.0f) return false;

	float centerY = bounds.y + bounds.height * 0.5f;
	Rectangle slopeBounds = slopeObject->GetBounds();
	if (centerY < slopeBounds.y || centerY > slopeBounds.y + slopeBounds.height) return false;

	const bool movingRight = dx > 0.0f;

	Vector2 sample = slopeObject->SampleLeftRightAtY(centerY);
	const float left = sample.x;
	const float right = sample.y;

	float leadingX, targetX, farBound;
	if (movingRight) { leadingX = bounds.x + bounds.width; targetX = left;  farBound = right; }
	else             { leadingX = bounds.x;                 targetX = right; farBound = left;  }
	const float gap = movingRight ? (targetX - leadingX) : (leadingX - targetX);

	constexpr float kFloatSlack = 0.01f;

	float t;
	if (gap <= 0.0f)
	{
		const bool stillWithinTile = movingRight ? (leadingX <= farBound + kFloatSlack)
		                                          : (leadingX >= farBound - kFloatSlack);
		if (!stillWithinTile) return false;
		t = 0.0f;
	}
	else t = gap / fabsf(dx);

	if (t < 0.0f || t > 1.0f) return false;

	tOut = t;
	return true;
}

bool PhysicsManager::CheckAndResolveCollisionXSlope(LevelObject* slopeObject, LevelObject* movingObject, bool blockedByRealSlopes, float prevVelocityX)
{
	// Failsafe but should not happen if the function is called correctly
	if (!slopeObject->HasActualSlopedHitbox()) return false;

	// Entities are not meant to be blocked by slopes in X movement
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
		movingObject->CollidedWithX(slopeObject, prevVelocityX);
        slopeObject->CollidedWithX(movingObject, prevVelocityX);
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
		movingObject->CollidedWithX(slopeObject, prevVelocityX);
        slopeObject->CollidedWithX(movingObject, prevVelocityX);
        return true;
    }

    return false;
}

void PhysicsManager::MoveAndResolveCollisionsY(const std::vector<LevelObject*>& objects, const float deltaTime, bool checkSemisolidPartialBottom, bool affectedByEntitiesTop)
{
	for (LevelObject* o : objects)
	{
		float vel = o->GetVelocityY();
		if (vel == 0.0f) continue;

		Rectangle startBounds = o->GetBounds();

		// We sweep against all non-sloped tiles to find the proportion of dy (tMin) of the first collision.
		const float dy = vel * deltaTime;
		float tMin = 1.0f;
		bool hitFound = false;
		bool willGround = false;
		LevelObject* hitObject = nullptr;

		for (LevelObject* solid : _solidObjects)
		{
			float t;
			if (SweepY(startBounds, dy, solid, t) && t < tMin) { tMin = t; hitFound = true; willGround = dy > 0.0f; hitObject = solid; }
		}

		for (LevelObject* slope : _solidMaybeSlopedObjects)
		{
			float t;
			if (slope->HasActualSlopedHitbox()) { if (SweepYSlope(startBounds, dy, slope, t) && t < tMin) { tMin = t; hitFound = true; willGround = dy > 0.0f; hitObject = slope; } }
			else { if (SweepY(startBounds, dy, slope, t) && t < tMin) { tMin = t; hitFound = true; willGround = dy > 0.0f; hitObject = slope; } }
		}

		for (LevelObject* breakable : _solidBreakableObjects)
		{
			if (breakable->GetBreakableIsBroken()) continue;

			float t;
			if (!SweepY(startBounds, dy, breakable, t)) continue;

			if (o->GetVelocityMagnitude() >= breakable->GetBreakableBreakSpeed())
			{
				breakable->BreakableBreak(o);
				continue; // Ignore collision if it goes fast enough to break it
			}

			if (t < tMin) { tMin = t; hitFound = true; willGround = dy > 0.0f; hitObject = breakable; }
		}

		for (LevelObject* semisolidTotal : _semisolidTotalObjects)
		{
			float t;
			if (SweepYOnlyFromTop(startBounds, dy, semisolidTotal, t) && t < tMin) { tMin = t; hitFound = true; willGround = true; hitObject = semisolidTotal; }
		}

		for (LevelObject* semisolidPartial : _semisolidPartialObjects)
		{
			float t;
			if (checkSemisolidPartialBottom) {
				if (SweepY(startBounds, dy, semisolidPartial, t) && t < tMin) { tMin = t; hitFound = true; willGround = true; hitObject = semisolidPartial; }
			} else {
				if (SweepYOnlyFromTop(startBounds, dy, semisolidPartial, t) && t < tMin) { tMin = t; hitFound = true; willGround = true; hitObject = semisolidPartial; }
			}
		}

		if (affectedByEntitiesTop)
		{
			for (LevelObject* entity : _entityObjects)
			{
				if (entity == o) continue;
				float t;
				if (SweepYOnlyFromTop(startBounds, dy, entity, t) && t < tMin) { tMin = t; hitFound = true; willGround = true; hitObject = entity; }
			}
		}

		// Move the object to the new position based on tMin
		o->SetBoundsY(startBounds.y + dy * tMin);
		if (hitFound)
		{
			o->SetVelocityY(0.0f);
			if (willGround) o->SetIsGrounded(true);

			o->CollidedWithY(hitObject, vel);
			if (hitObject) hitObject->CollidedWithY(o, vel);
		}

		// After moving, we check for collisions with actually sloped tiles and resolve them if necessary.
		// This is done after the initial move to ensure that we handle slopes correctly, especially if the object is moving into a slope.
		Rectangle bounds = o->GetBounds();
		for (LevelObject* solidMaybeSloped : _solidMaybeSlopedObjects)
		{
			if (!solidMaybeSloped->HasActualSlopedHitbox()) continue;
			if (CheckAndResolveCollisionYSlope(solidMaybeSloped, o, vel)) bounds = o->GetBounds();
		}
	}
}

bool PhysicsManager::SweepY(const Rectangle& bounds, float dy, LevelObject* stillObject, float& tOut)
{
	if (dy == 0.0f) return false;

	// Check horizontal overlap first
	Rectangle stillBounds = stillObject->GetBounds();
	if (bounds.x + bounds.width <= stillBounds.x || bounds.x >= stillBounds.x + stillBounds.width) return false;

	// Discard collision if it comes from a side that the still object does not have as solid
	const bool movingDown = dy > 0.0f;
	const uint8_t relevantSide = movingDown ? SOLID_SIDE_TOP : SOLID_SIDE_BOTTOM;
	if (!stillObject->GetSolidSideCollisionMask(relevantSide)) return false;

	// Find the edges that might collide and get their gap
	float leadingY, targetY;
	if (movingDown) { leadingY = bounds.y + bounds.height; targetY = stillBounds.y; }
	else            { leadingY = bounds.y;                 targetY = stillBounds.y + stillBounds.height; }
	const float gap = movingDown ? (targetY - leadingY) : (leadingY - targetY);

	// kPenetrationEpsilon is a small value to account for floating-point inaccuracies and prevent objects from getting stuck when they are very close to each other
	// However, if the still object is a ramp, we want to allow a larger penetration epsilon to avoid getting stuck on ramps
	const bool nearRamp = !stillObject->GetSolidSideCollisionMask(SOLID_SIDE_LEFT) ||
	                       !stillObject->GetSolidSideCollisionMask(SOLID_SIDE_RIGHT);
	const float penetrationEpsilon = nearRamp ? 1.0f : 0.01f;

	// If the gap is negative, it means we are already overlapping. If it's positive, we can calculate the time of collision
	float t;
	if (gap <= 0.0f)
	{
		if (gap < -penetrationEpsilon) return false;
		t = 0.0f;
	}
	else t = gap / fabsf(dy);

	// If t is not between 0 and 1 the objects will not collide this frame
	if (t < 0.0f || t > 1.0f) return false;

	tOut = t;
	return true;
}

bool PhysicsManager::SweepYSlope(const Rectangle& bounds, float dy, LevelObject* slopeObject, float& tOut)
{
    if (dy == 0.0f) return false;

    // Check horizontal overlap with center of object first
    float centerX = bounds.x + bounds.width * 0.5f;
    Rectangle slopeBounds = slopeObject->GetBounds();
    if (centerX < slopeBounds.x || centerX > slopeBounds.x + slopeBounds.width) return false;

    const bool movingDown = dy > 0.0f;

    Vector2 sample = slopeObject->SampleTopBottomAtX(centerX);
    const float top = sample.x;
    const float bottom = sample.y;

    // Find the edges that might collide and get their gap
    float leadingY, targetY;
    if (movingDown) { leadingY = bounds.y + bounds.height; targetY = top; }
    else            { leadingY = bounds.y;                 targetY = bottom; }
    const float gap = movingDown ? (targetY - leadingY) : (leadingY - targetY);

    // kPenetrationEpsilon is a small value to account for floating-point inaccuracies and prevent objects from getting stuck when they are very close to each other
    // However, if the still object is a ramp, we want to allow a larger penetration epsilon to avoid getting stuck on ramps
    const bool nearRamp = !slopeObject->GetSolidSideCollisionMask(SOLID_SIDE_LEFT) ||
                           !slopeObject->GetSolidSideCollisionMask(SOLID_SIDE_RIGHT);
    const float penetrationEpsilon = nearRamp ? 0.5f : 0.01f;

    // If the gap is negative, it means we are already overlapping. If it's positive, we can calculate the time of collision
    float t;
    if (gap <= 0.0f)
    {
        if (gap < -penetrationEpsilon) return false;
        t = 0.0f;
    }
    else t = gap / fabsf(dy);

    // If t is not between 0 and 1 the objects will not collide this frame
    if (t < 0.0f || t > 1.0f) return false;

    tOut = t;
    return true;
}

bool PhysicsManager::SweepYOnlyFromTop(const Rectangle& startBounds, float dy, LevelObject* stillObject, float& tOut)
{
	if (dy <= 0.0f) return false; // Only when falling

	// Check horizontal overlap first
	Rectangle stillBounds = stillObject->GetBounds();
	if (startBounds.x + startBounds.width <= stillBounds.x || startBounds.x >= stillBounds.x + stillBounds.width) return false;

	constexpr float kAboveEpsilon = 0.01f;
	const float startFeet = startBounds.y + startBounds.height;

	// Collision only occurs if the moving object was above the still object (with a small epsilon to account for floating-point inaccuracies)
	if (startFeet > stillBounds.y + kAboveEpsilon) return false;

	// Find the edges that might collide and get their gap and time of collision
	const float gap = stillBounds.y - startFeet;
	const float t = gap / dy;

	// If t is not between 0 and 1 the objects will not collide this frame
	if (t < 0.0f || t > 1.0f) return false;

	tOut = t;
	return true;
}

bool PhysicsManager::CheckAndResolveCollisionYSlope(LevelObject* slopeObject, LevelObject* movingObject, float prevVelocityY)
{
	if (!slopeObject->HasActualSlopedHitbox()) return false;

    Rectangle bounds = movingObject->GetBounds();
    Rectangle slopeBounds = slopeObject->GetBounds();

    float centerX = bounds.x + bounds.width * 0.5f;
    if (centerX < slopeBounds.x || centerX > slopeBounds.x + slopeBounds.width) return false;
    if (bounds.y + bounds.height < slopeBounds.y || bounds.y > slopeBounds.y + slopeBounds.height) return false;

    Vector2 sample = slopeObject->SampleTopBottomAtX(centerX);
    const float top = sample.x;
    const float bottom = sample.y;

    const float feetY = bounds.y + bounds.height;
    const float headY = bounds.y;

    const bool feetInSlice = feetY >= top && feetY <= bottom;
    const bool headInSlice = headY >= top && headY <= bottom;
    const bool swallowed   = top >= headY && bottom <= feetY;

    bool isFloor;
    if (swallowed) isFloor = (feetY - top) <= (bottom - headY);
    else if (feetInSlice) isFloor = true;
    else if (headInSlice) isFloor = false;
    else return false;

    if (isFloor)
    {
        movingObject->SetBoundsY(top - bounds.height);
        movingObject->SetIsGrounded(true);
    }
    else movingObject->SetBoundsY(bottom);

    movingObject->SetVelocityY(0.0f);
	movingObject->CollidedWithY(slopeObject, prevVelocityY);
    slopeObject->CollidedWithY(movingObject, prevVelocityY);
    return true;
}

void PhysicsManager::CheckOverlaps(const std::vector<LevelObject*>& listA, const std::vector<LevelObject*>& listB)
{
	for (LevelObject* a : listA)
	{
		Rectangle boundsA = a->GetBounds();
		for (LevelObject* b : listB)
		{
			if (a == b) continue;

			Rectangle boundsB = b->GetBounds();

			const bool overlap = CheckCollisionRecs(boundsA, boundsB);
			if (!overlap) continue;

			a->OnOverlap(b);
			b->OnOverlap(a);
		}
	}
}

void PhysicsManager::CheckOverlapsSelf(const std::vector<LevelObject*>& list)
{
	for (size_t i = 0; i < list.size(); ++i)
	{
		Rectangle boundsA = list[i]->GetBounds();
		for (size_t j = i + 1; j < list.size(); ++j)
		{
			if (!CheckCollisionRecs(boundsA, list[j]->GetBounds())) continue;

			list[i]->OnOverlap(list[j]);
			list[j]->OnOverlap(list[i]);
		}
	}
}

void PhysicsManager::RegisterObject(LevelObject* object)
{
	if (!object) return;

	HitboxType hitboxType = object->GetHitboxType();
	switch (hitboxType) 
	{
		case HitboxType::Solid:				_solidObjects.push_back(object); break;
		case HitboxType::SolidMaybeSloped:	_solidMaybeSlopedObjects.push_back(object); break;
		case HitboxType::SolidBreakable:	_solidBreakableObjects.push_back(object); break;
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
		case HitboxType::SolidBreakable:	RemoveObjectFromList(_solidBreakableObjects, object); break;
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

LevelObject* PhysicsManager::GetDugObjectAt(const Vector2& position, LevelObject* digger) const
{
	// In theory, order does not matter since static objects do not overlap vertically
	// But since entities can move, it makes sense to check them first
	for (LevelObject* entity : _entityObjects)
	{
		if (CanObjectBeDug(entity, position, digger)) return entity;
	}

	for (LevelObject* solid : _solidObjects)
	{
		if (CanObjectBeDug(solid, position, digger)) return solid;
	}

	for (LevelObject* solidMaybeSloped : _solidMaybeSlopedObjects)
	{
		if (CanObjectBeDug(solidMaybeSloped, position, digger)) return solidMaybeSloped;
	}

	for (LevelObject* solidBreakable : _solidBreakableObjects)
	{
		if (CanObjectBeDug(solidBreakable, position, digger)) return solidBreakable;
	}

	for (LevelObject* semisolidTotal : _semisolidTotalObjects)
	{
		if (CanObjectBeDug(semisolidTotal, position, digger)) return semisolidTotal;
	}

	for (LevelObject* semisolidPartial : _semisolidPartialObjects)
	{
		if (CanObjectBeDug(semisolidPartial, position, digger)) return semisolidPartial;
	}

	return nullptr;
}

bool PhysicsManager::CanObjectBeDug(LevelObject* object, const Vector2& position, LevelObject* digger) const
{
	if (object == digger) return false;

	const Rectangle bounds = object->GetBounds();
	if (position.x < bounds.x || position.x > bounds.x + bounds.width) return false;

	// To check if we're digging from the top, we check the position with a small epsilon above and below
	constexpr float kDigVerticalTolerance = 0.1f;
	const float digYTop    = position.y - kDigVerticalTolerance;
	const float digYBottom = position.y + kDigVerticalTolerance;

	// This accounts for slopes. Nice!
	const Vector2 objectBoundsTopBottom = object->SampleTopBottomAtX(position.x);
	const float objectTop = objectBoundsTopBottom.x;

	return (digYTop <= objectTop && digYBottom >= objectTop);
}