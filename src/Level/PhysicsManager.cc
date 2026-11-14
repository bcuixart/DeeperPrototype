#include "PhysicsManager.hh"
#include "Level.hh"
#include "LevelObject.hh"

PhysicsManager::PhysicsManager(Level* level) : _level(level)
{

}

PhysicsManager::~PhysicsManager()
{
	
}

void PhysicsManager::Update(const float deltaTime)
{
	// Reset grounded state for all objects
	for (LevelObject* o : _entityObjects) o->SetWasGroundedBeforePhysicsUpdate();
	for (LevelObject* o : _dynamicObjects) o->SetWasGroundedBeforePhysicsUpdate();
	for (LevelObject* o : _entityObjects) o->SetIsGrounded(false, SurfaceType::None);
	for (LevelObject* o : _dynamicObjects) o->SetIsGrounded(false, SurfaceType::None);

	// Apply gravity
	ApplyGravity(_entityObjects, deltaTime);
	ApplyGravity(_dynamicObjects, deltaTime);

	// Apply force fields
	ApplyForceFields(_entityObjects, deltaTime);
	ApplyForceFields(_dynamicObjects, deltaTime);

	// Move objects and resolve collisions
	MoveAndResolveMovement(_entityObjects, deltaTime, false, false, false, true);
	MoveAndResolveMovement(_dynamicObjects, deltaTime, true, true, true, false);

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
	for (LevelObject* o : objects) if (!o->IsInTerrain()) o->AddVelocityY(kGravity * deltaTime);
}

void PhysicsManager::ApplyForceFields(const std::vector<LevelObject*>& objects, const float deltaTime)
{
	for (LevelObject* o : objects)
	{
		if (o->IsInTerrain()) continue;

		Rectangle bounds = o->GetBounds();
		for (LevelObject* field : _forceFieldObjects)
		{
			if (!field->GetForceFieldIsActive()) continue;
			if (!CheckCollisionRecs(bounds, field->GetBounds())) continue;

			Vector2 force = field->GetForceFieldForce();
			o->AddVelocityX(force.x * deltaTime);
			o->AddVelocityY(force.y * deltaTime);

			o->OnForceField(field);
		}
	}
}

void PhysicsManager::MoveAndResolveMovement(const std::vector<LevelObject*>& objects, const float deltaTime, 
	bool affectedByPartialSemisolidsX, bool blockedByRealSlopesX, bool checkSemisolidPartialBottom, bool affectedByEntitiesTop)
{
	for (LevelObject* o : objects)
	{
		if (o->IsInTerrain()) { MoveAndResolveObjectInTerrain(o, deltaTime); continue; }

		// Substeps only for entities: their move is split in dependent phases (X, Y, slope snap)
		int steps = 1;
		if (!blockedByRealSlopesX)
		{
			const float kMaxMoveStep = 0.2f;
			const float maxMove = std::max(fabsf(o->GetVelocityX()), fabsf(o->GetVelocityY())) * deltaTime;
			steps = std::max(1, (int)std::ceil(maxMove / kMaxMoveStep));
		}
		const float stepDeltaTime = deltaTime / steps;

		for (int i = 0; i < steps; ++i)
		{
			if (o->IsInTerrain()) break; // A previous substep callback may have sent it into terrain
			const float stepStartX = o->GetBounds().x;

			MoveAndResolveCollisionsX(o, stepDeltaTime, affectedByPartialSemisolidsX, blockedByRealSlopesX);
			if (o->IsInTerrain()) break;

			const float groundMargin = (!blockedByRealSlopesX) ? fabsf(o->GetBounds().x - stepStartX) + kGroundTolerance : -1.0f;
			if (MoveAndResolveCollisionsY(o, stepDeltaTime, checkSemisolidPartialBottom, affectedByEntitiesTop, groundMargin))
			{
				o->SetBoundsX(stepStartX);
				o->SetVelocityX(0.0f);
				break;
			}
		}
	}
}

void PhysicsManager::MoveAndResolveCollisionsX(LevelObject* object, const float deltaTime, bool affectedByPartialSemisolids, bool blockedByRealSlopes)
{
	float vel = object->GetVelocityX();
	if (vel == 0.0f) return;

	Rectangle bounds = object->GetBounds();

	// We sweep against all non-sloped tiles to find the proportion of dx (tMin) of the first collision.
	const float dx = vel * deltaTime;
	float tMin = 1.0f;
	bool hitFound = false;
	LevelObject* hitObject = nullptr;

	for (LevelObject* solid : _solidObjects)
	{
		float t;
		if (SweepX(bounds, dx, solid, t) && t < tMin) { tMin = t; hitFound = true; hitObject = solid; }
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

	std::vector<LevelObject*> breakablesToBreak;
	std::vector<float> breakablesT;
	for (LevelObject* breakable : _solidBreakableObjects)
	{
		if (breakable->GetBreakableIsBroken()) continue;

		float t;
		if (!SweepX(bounds, dx, breakable, t)) continue;

		if (fabs(object->GetVelocityX()) >= object->GetInteractionVelocityX())
		{
			breakablesToBreak.push_back(breakable);
			breakablesT.push_back(t);
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

	// Break the breakables that were hit by the moving object
	for (int i = 0; i < (int)breakablesToBreak.size(); ++i)
		if (breakablesT[i] <= tMin) breakablesToBreak[i]->BreakableBreak(object);

	// Move the object to the new position based on tMin
	object->SetBoundsX(bounds.x + dx * tMin);
	if (hitFound)
	{
		object->SetVelocityX(0.0f);
		if (hitObject) hitObject->CollidedWithX(object, vel);
		object->CollidedWithX(hitObject, vel);
	}

	// After moving, we check for collisions with actually sloped tiles and resolve them if necessary.
	// This is done after the initial move to ensure that we handle slopes correctly, especially if the object is moving into a slope.
	for (LevelObject* solidMaybeSloped : _solidMaybeSlopedObjects)
	{
		if (!solidMaybeSloped->HasActualSlopedHitbox()) continue;
		CheckAndResolveCollisionXSlope(solidMaybeSloped, object, affectedByPartialSemisolids, blockedByRealSlopes, vel);
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

	// Check vertical overlap with center of object first
	float centerY = bounds.y + bounds.height * 0.5f;
	Rectangle slopeBounds = slopeObject->GetBounds();
	if (centerY < slopeBounds.y || centerY > slopeBounds.y + slopeBounds.height) return false;

	const bool movingRight = dx > 0.0f;

	Vector2 sample = slopeObject->SampleLeftRightAtY(centerY);
	const float left = sample.x;
	const float right = sample.y;

	// Find the edges that might collide and get their gap
	float leadingX, targetX;
	if (movingRight) { leadingX = bounds.x + bounds.width; targetX = left; }
	else             { leadingX = bounds.x;                 targetX = right; }
	const float gap = movingRight ? (targetX - leadingX) : (leadingX - targetX);

	// kPenetrationEpsilon is a small value to account for floating-point inaccuracies and prevent objects from getting stuck when they are very close to each other
	const float penetrationEpsilon = 0.5f;

	// If the gap is negative, it means we are already overlapping. If it's positive, we can calculate the time of collision
	float t;
	if (gap <= 0.0f)
	{
		if (gap < -penetrationEpsilon) return false;
		t = 0.0f;
	}
	else t = gap / fabsf(dx);

	// If t is not between 0 and 1 the objects will not collide this frame
	if (t < 0.0f || t > 1.0f) return false;

	tOut = t;
	return true;
}

bool PhysicsManager::CheckAndResolveCollisionXSlope(LevelObject* slopeObject, LevelObject* movingObject, bool affectedByPartialSemisolidsX, bool blockedByRealSlopes, float prevVelocityX)
{
	// Failsafe but should not happen if the function is called correctly
	if (!slopeObject->HasActualSlopedHitbox()) return false;

	// Entities are not meant to be blocked by slopes in X movement
	if (!blockedByRealSlopes) return false;

	Rectangle bounds = movingObject->GetBounds();
	Rectangle slopeBounds = slopeObject->GetBounds();

	constexpr float kMinVerticalOverlap = 0.01f;
	if (bounds.y + bounds.height <= slopeBounds.y + kMinVerticalOverlap ||
		bounds.y >= slopeBounds.y + slopeBounds.height - kMinVerticalOverlap) return false;
	if (bounds.x + bounds.width < slopeBounds.x || bounds.x > slopeBounds.x + slopeBounds.width) return false;

	float topY    = Clamp(bounds.y,                slopeBounds.y, slopeBounds.y + slopeBounds.height);
	float bottomY = Clamp(bounds.y + bounds.height, slopeBounds.y, slopeBounds.y + slopeBounds.height);

	Vector2 sampleTop    = slopeObject->SampleLeftRightAtY(topY);
	Vector2 sampleBottom = slopeObject->SampleLeftRightAtY(bottomY);

	const float rightEdge = bounds.x + bounds.width;
	const float leftEdge  = bounds.x;

	const bool rightInTop    = rightEdge >= sampleTop.x    && rightEdge <= sampleTop.y;
	const bool rightInBottom = rightEdge >= sampleBottom.x && rightEdge <= sampleBottom.y;
	const bool leftInTop     = leftEdge  >= sampleTop.x    && leftEdge  <= sampleTop.y;
	const bool leftInBottom  = leftEdge  >= sampleBottom.x && leftEdge  <= sampleBottom.y;

	float proposedX;
	if (rightInTop || rightInBottom)
	{
		float leftBoundary = 0.0f;
		if (rightInTop)    leftBoundary = sampleTop.x;
		if (rightInBottom) leftBoundary = rightInTop ? fminf(leftBoundary, sampleBottom.x) : sampleBottom.x;

		proposedX = leftBoundary - bounds.width;
	}
	else if (leftInTop || leftInBottom)
	{
		float rightBoundary = 0.0f;
		if (leftInTop)    rightBoundary = sampleTop.y;
		if (leftInBottom) rightBoundary = leftInTop ? fmaxf(rightBoundary, sampleBottom.y) : sampleBottom.y;

		proposedX = rightBoundary;
	}
	else return false;

	// Before committing the snap, make sure it wouldn't tunnel the object through another solid.
	LevelObject* blocker = FindBlockingSolidX(movingObject, bounds, proposedX, affectedByPartialSemisolidsX);
	LevelObject* collidedWith = blocker ? blocker : slopeObject;

	if (!blocker) movingObject->SetBoundsX(proposedX);

	movingObject->SetVelocityX(0.0f);
	collidedWith->CollidedWithX(movingObject, prevVelocityX);
	movingObject->CollidedWithX(collidedWith, prevVelocityX);
	return true;
}

bool PhysicsManager::MoveAndResolveCollisionsY(LevelObject* object, const float deltaTime, bool checkSemisolidPartialBottom, bool affectedByEntitiesTop, float groundMargin)
{
	float vel = object->GetVelocityY();
	if (vel == 0.0f) return ResolveSlopesY(object, 0.0f, groundMargin);

	Rectangle startBounds = object->GetBounds();

	// We sweep against all non-sloped tiles to find the proportion of dy (tMin) of the first collision.
	const float dy = vel * deltaTime;
	float tMin = 1.0f;
	bool hitFound = false;
	bool willGround = false;
	bool willBounceTrampoline = false;
	LevelObject* hitObject = nullptr;

	for (LevelObject* solid : _solidObjects)
	{
		float t;
		if (SweepY(startBounds, dy, solid, t) && t < tMin) { tMin = t; hitFound = true; willGround = dy > 0.0f; willBounceTrampoline = false; hitObject = solid; }
	}

	for (LevelObject* slope : _solidMaybeSlopedObjects)
	{
		float t;
		if (slope->HasActualSlopedHitbox()) { if (SweepYSlope(startBounds, dy, slope, t) && t < tMin) { tMin = t; hitFound = true; willGround = dy > 0.0f; willBounceTrampoline = false; hitObject = slope; } }
		else { if (SweepY(startBounds, dy, slope, t) && t < tMin) { tMin = t; hitFound = true; willGround = dy > 0.0f; willBounceTrampoline = false; hitObject = slope; } }
	}

	std::vector<LevelObject*> breakablesToBreak;
	std::vector<float> breakablesT;
	for (LevelObject* breakable : _solidBreakableObjects)
	{
		if (breakable->GetBreakableIsBroken()) continue;

		float t;
		if (!SweepY(startBounds, dy, breakable, t)) continue;

		if (fabsf(object->GetVelocityY()) >= object->GetInteractionVelocityY())
		{
			breakablesToBreak.push_back(breakable);
			breakablesT.push_back(t);
			continue; // Ignore collision if it goes fast enough to break it
		}

		if (t < tMin) { tMin = t; hitFound = true; willGround = dy > 0.0f; willBounceTrampoline = false; hitObject = breakable; }
	}

	for (LevelObject* semisolidTotal : _semisolidTotalObjects)
	{
		float t;
		if (SweepYOnlyFromTop(startBounds, dy, semisolidTotal, t) && t < tMin) { tMin = t; hitFound = true; willGround = true; willBounceTrampoline = false; hitObject = semisolidTotal; }
	}

	for (LevelObject* semisolidPartial : _semisolidPartialObjects)
	{
		float t;
		if (checkSemisolidPartialBottom) {
			if (SweepY(startBounds, dy, semisolidPartial, t) && t < tMin) { tMin = t; hitFound = true; willGround = dy > 0.0f; willBounceTrampoline = false; hitObject = semisolidPartial; }
		} else {
			if (SweepYOnlyFromTop(startBounds, dy, semisolidPartial, t) && t < tMin) { tMin = t; hitFound = true; willGround = true; willBounceTrampoline = false; hitObject = semisolidPartial; }
		}
	}

	if (affectedByEntitiesTop)
	{
		for (LevelObject* entity : _entityObjects)
		{
			if (entity == object) continue;
			float t;
			if (SweepYOnlyFromTop(startBounds, dy, entity, t) && t < tMin) { tMin = t; hitFound = true; willGround = true; willBounceTrampoline = false; hitObject = entity; }
		}
	}

	for (LevelObject* trampoline : _trampolineObjects)
	{
		float t;
		if (SweepYOnlyFromTop(startBounds, dy, trampoline, t) && t < tMin) { tMin = t; hitFound = true; willGround = false; willBounceTrampoline = true; hitObject = trampoline; }
	}

	// Break the breakables that were hit by the moving object
	for (int i = 0; i < (int)breakablesToBreak.size(); ++i)
		if (breakablesT[i] <= tMin) breakablesToBreak[i]->BreakableBreak(object);

	// Move the object to the new position based on tMin
	object->SetBoundsY(startBounds.y + dy * tMin);
	if (hitFound)
	{
		object->SetVelocityY(0.0f);
		if (willBounceTrampoline) 
		{ 
			if (hitObject) hitObject->CollidedWithY(object, vel);
			object->OnTrampoline(hitObject, vel);
		}
		else
		{
			if (willGround)
			{
				object->SetIsGrounded(true, (hitObject ? hitObject->GetSurfaceType() : SurfaceType::FlatGround));

				// Snap to surface. This fixes slightly clipping into flat ground after going up a slope.
				if (tMin <= 0.0f && hitObject && !hitObject->HasActualSlopedHitbox())
				{
					object->SetBoundsY(hitObject->GetBounds().y - startBounds.height);
				}
			}

			if (hitObject) hitObject->CollidedWithY(object, vel);
			object->CollidedWithY(hitObject, vel);
		}
	}

	// After moving, we check for collisions with actually sloped tiles and resolve them if necessary.
	// This is done after the initial move to ensure that we handle slopes correctly, especially if the object is moving into a slope.
	return ResolveSlopesY(object, vel, groundMargin);
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
	const float centerX = bounds.x + bounds.width * 0.5f;
	const bool centerOver = centerX >= stillBounds.x && centerX <= stillBounds.x + stillBounds.width;
	const float penetrationEpsilon = (nearRamp && centerOver) ? 1.0f : 0.01f;

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

	// If the gap is negative, it means we are already overlapping. If it's positive, we can calculate the time of collision
	if (gap <= 0.0f) { tOut = 0.0f; return true; }

	// If t is not between 0 and 1 the objects will not collide this frame
	if (t < 0.0f || t > 1.0f) return false;

	tOut = t;
	return true;
}

bool PhysicsManager::CheckAndResolveCollisionYSlope(LevelObject* slopeObject, LevelObject* movingObject, float prevVelocityY, float groundMargin, bool wasGrounded, bool& outCancelWholeMove)
{
	if (!slopeObject->HasActualSlopedHitbox()) return false;

	const bool useGround = groundMargin >= 0.0f;

	Rectangle bounds = movingObject->GetBounds();
	Rectangle slopeBounds = slopeObject->GetBounds();

	float centerX = bounds.x + bounds.width * 0.5f;
	if (centerX < slopeBounds.x || centerX > slopeBounds.x + slopeBounds.width) return false;
	const float slack = useGround ? groundMargin : 0.0f;
	if (bounds.y + bounds.height < slopeBounds.y - slack || bounds.y > slopeBounds.y + slopeBounds.height) return false;

	Vector2 sample = slopeObject->SampleTopBottomAtX(centerX);
	const float top = sample.x;
	const float bottom = sample.y;

	const float feetY = bounds.y + bounds.height;
	const float headY = bounds.y;

	bool groundSnap = false;
	if (useGround)
	{
		const float depth = feetY - top;
		const bool embedded = depth > 0.0f && depth <= groundMargin;
		const bool stick    = depth <= 0.0f && wasGrounded && prevVelocityY >= 0.0f && -depth <= groundMargin;
		groundSnap = embedded || stick;
	}

	bool isFloor;
	if (groundSnap) isFloor = true;
	else
	{
		const bool feetInSlice = feetY >= top && feetY <= bottom;
		const bool headInSlice = headY >= top && headY <= bottom;
		const bool swallowed   = top >= headY && bottom <= feetY;

		if (swallowed) isFloor = (feetY - top) <= (bottom - headY);
		else if (feetInSlice) isFloor = true;
		else if (headInSlice) isFloor = false;
		else return false;
	}

	const float proposedY = isFloor ? (top - bounds.height) : bottom;

	// Before committing the snap, make sure it wouldn't tunnel the object through another solid.
	LevelObject* blocker = FindBlockingSolidY(movingObject, bounds, proposedY, isFloor);
	if (blocker)
	{
		movingObject->SetVelocityY(0.0f);
		if (isFloor) movingObject->SetIsGrounded(true, blocker->GetSurfaceType());

		blocker->CollidedWithY(movingObject, prevVelocityY);
		movingObject->CollidedWithY(blocker, prevVelocityY);

		outCancelWholeMove = true;
		return true;
	}

	// Rising object pushed up by a slope (jumping while moving up a ramp): reposition but keep its upward velocity.
	if (groundSnap && prevVelocityY < 0.0f)
	{
		movingObject->SetBoundsY(proposedY);
		return true;
	}

	movingObject->SetBoundsY(proposedY);
	if (isFloor) movingObject->SetIsGrounded(true, slopeObject->GetSurfaceType());

	movingObject->SetVelocityY(0.0f);
	slopeObject->CollidedWithY(movingObject, prevVelocityY);
	movingObject->CollidedWithY(slopeObject, prevVelocityY);
	return true;
}

bool PhysicsManager::ResolveSlopesY(LevelObject* object, float prevVelocityY, float groundMargin)   // returns true if the whole move must be canceled
{
	const bool wasGrounded = object->GetWasGroundedBeforePhysicsUpdate();
	for (LevelObject* solidMaybeSloped : _solidMaybeSlopedObjects)
	{
		if (!solidMaybeSloped->HasActualSlopedHitbox()) continue;

		bool cancelWholeMove = false;
		if (CheckAndResolveCollisionYSlope(solidMaybeSloped, object, prevVelocityY, groundMargin, wasGrounded, cancelWholeMove))
		{
			if (cancelWholeMove) return true;
		}
	}
	return false;
}

LevelObject* PhysicsManager::FindBlockingSolidY(const LevelObject* ignore, const Rectangle& bounds, float proposedY, bool movingDown) const
{
	const float centerX = bounds.x + bounds.width * 0.5f;

	auto overlaps = [&](const Rectangle& b)
	{
		if (centerX <= b.x || centerX >= b.x + b.width) return false;
		return proposedY < b.y + b.height && proposedY + bounds.height > b.y;
	};

	for (LevelObject* o : _solidObjects)
	{
		if (o != ignore && overlaps(o->GetBounds())) 
		{
			if (movingDown && !o->GetSolidSideCollisionMask(SOLID_SIDE_TOP)) continue;
			else if (!movingDown && !o->GetSolidSideCollisionMask(SOLID_SIDE_BOTTOM)) continue;
			//printf("Found blocking solid Y: %p\n", o);
			//printf("Dog bounds: (%f, %f, %f, %f), proposedY: %f\n", bounds.x, bounds.y, bounds.width, bounds.height, proposedY);
			//printf("Collided object bounds: (%f, %f, %f, %f)\n", o->GetBounds().x, o->GetBounds().y, o->GetBounds().width, o->GetBounds().height);
			return o;
		}
	}

	for (LevelObject* o : _solidBreakableObjects)
		if (o != ignore && !o->GetBreakableIsBroken() && overlaps(o->GetBounds())) 
		{
			//printf("Found blocking solid breakable Y: %p\n", o);
			return o;
		}

	if (movingDown)
	{
		constexpr float kAboveEpsilon = 0.01f;
		for (LevelObject* o : _semisolidTotalObjects)
		{
			if (o == ignore) continue;
			const Rectangle b = o->GetBounds();
			if (bounds.x + bounds.width <= b.x || bounds.x >= b.x + b.width) continue;
			if (bounds.y + bounds.height > b.y + kAboveEpsilon) continue;
			if (proposedY + bounds.height > b.y) 
			{
				//printf("Found blocking semisolid total Y: %p\n", o);
				return o;
			}
		}
		for (LevelObject* o : _semisolidPartialObjects)
		{
			if (o == ignore) continue;
			const Rectangle b = o->GetBounds();
			if (bounds.x + bounds.width <= b.x || bounds.x >= b.x + b.width) continue;
			if (bounds.y + bounds.height > b.y + kAboveEpsilon) continue;
			if (proposedY + bounds.height > b.y) 
			{
				//printf("Found blocking semisolid partial Y: %p\n", o);
				return o;
			}
		}
	}

	return nullptr;
}

LevelObject* PhysicsManager::FindBlockingSolidX(const LevelObject* ignore, const Rectangle& bounds, float proposedX, bool affectedByPartialSemisolidsX) const
{
	auto overlaps = [&](const Rectangle& b)
	{
		if (bounds.y + bounds.height <= b.y || bounds.y >= b.y + b.height) return false;
		return proposedX < b.x + b.width && proposedX + bounds.width > b.x;
	};

	const bool movingRight = proposedX > bounds.x;

	for (LevelObject* o : _solidObjects)
	{
		if (o != ignore && overlaps(o->GetBounds()))
		{
			if (movingRight && !o->GetSolidSideCollisionMask(SOLID_SIDE_LEFT)) continue;
			else if (!movingRight && !o->GetSolidSideCollisionMask(SOLID_SIDE_RIGHT)) continue;
			//printf("Found blocking solid X: %p\n", o);
			return o;
		}
	}

	for (LevelObject* o : _solidBreakableObjects)
		if (o != ignore && !o->GetBreakableIsBroken() && overlaps(o->GetBounds()))
		{
			//printf("Found blocking solid breakable X: %p\n", o);
			return o;
		}

	if (affectedByPartialSemisolidsX)
	{
		for (LevelObject* o : _semisolidPartialObjects)
			if (o != ignore && overlaps(o->GetBounds()))
			{
				//printf("Found blocking semisolid partial X: %p\n", o);
				return o;
			}
	}

	return nullptr;
}

void PhysicsManager::MoveAndResolveObjectInTerrain(LevelObject* object, const float deltaTime)
{
	if (!object || !object->IsInTerrain()) return;

	int8_t currentTileX, currentTileY;
	object->GetTerrainMoveTarget(currentTileX, currentTileY);

	if (_level->IsObjectCenteredOnTerrainTile(object, currentTileX, currentTileY))
	{
		// Check velocity
		float velX = object->GetVelocityX();
		float velY = object->GetVelocityY();

		// If the object does not have to move, do nothing.
		if (velX == 0.0f && velY == 0.0f) return;

		// Get the angle and movement deltas.
		constexpr float kTan30 = 0.57735027f;
		const float ax = fabsf(velX), ay = fabsf(velY);
		int8_t angleDeltaX = (ax >= ay * kTan30) ? (velX > 0.0f ? 1 : -1) : 0;
		int8_t angleDeltaY = (ay >= ax * kTan30) ? (velY > 0.0f ? 1 : -1) : 0;

		int8_t deltaX, deltaY;
		_level->GetTerrainMovementDelta(currentTileX, currentTileY, currentTileX + angleDeltaX, currentTileY + angleDeltaY, deltaX, deltaY, ay > ax);

		object->SetTerrainMoveTarget(currentTileX + deltaX, currentTileY + deltaY);

		// Velocity always decays even when not moving to a new tile.
		float newVelX = velX * kTerrainVelocityDecay;
		float newVelY = velY * kTerrainVelocityDecay;
		
		object->SetVelocityX(fabs(newVelX) < kTerrainVelocityDecayLimit ? 0.0f : newVelX);
		object->SetVelocityY(fabs(newVelY) < kTerrainVelocityDecayLimit ? 0.0f : newVelY);
	}
	else
	{
		// currentTileX and Y are the target, despite the name here
		MoveObjectInTerrain(object, currentTileX, currentTileY, deltaTime);
	}
}

void PhysicsManager::MoveObjectInTerrain(LevelObject* object, uint8_t tX, uint8_t tY, const float deltaTime)
{	
	const Rectangle b = object->GetBounds();
	const float destX = tX + 0.5f - b.width * 0.5f;
	const float destY = tY + 0.5f - b.height * 0.5f;
	const float dx = destX - b.x, dy = destY - b.y;
	const float distance = sqrtf(dx * dx + dy * dy);

	float speed = object->GetTerrainDesiredMoveSpeed();
	if (speed <= 0.0f)
	{
		const float vx = object->GetVelocityX(), vy = object->GetVelocityY();
		speed = std::max(kTerrainMoveMinVelocity, sqrtf(vx * vx + vy * vy));
	}

	const float step = speed * deltaTime;
	if (step >= distance) { object->SetBoundsX(destX); object->SetBoundsY(destY); }
	else { const float k = step / distance; object->SetBoundsX(b.x + dx * k); object->SetBoundsY(b.y + dy * k); }
}

void PhysicsManager::CheckOverlaps(const std::vector<LevelObject*>& listA, const std::vector<LevelObject*>& listB)
{
	for (LevelObject* a : listA)
	{
		Rectangle boundsA = a->GetBounds();
		for (LevelObject* b : listB)
		{
			if (a == b) continue;
			if (a->IsInTerrain() != b->IsInTerrain()) continue;

			Rectangle boundsB = b->GetBounds();

			if (!CheckCollisionRecs(boundsA, boundsB)) continue;

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
			if (list[i]->IsInTerrain() != list[j]->IsInTerrain()) continue;

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
		case HitboxType::Trampoline:		_trampolineObjects.push_back(object); break;
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
		case HitboxType::Trampoline:		RemoveObjectFromList(_trampolineObjects, object); break;
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

	// Slopes must be checked before regular solids
	for (LevelObject* solidMaybeSloped : _solidMaybeSlopedObjects)
	{
		if (CanObjectBeDug(solidMaybeSloped, position, digger)) return solidMaybeSloped;
	}

	for (LevelObject* solid : _solidObjects)
	{
		if (CanObjectBeDug(solid, position, digger)) return solid;
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
	if (object == nullptr) return false;

	if (object->IsInTerrain()) return false;

	const Rectangle bounds = object->GetBounds();
	if (position.x < bounds.x || position.x > bounds.x + bounds.width) return false;

	if (object->HasActualSlopedHitbox()) return CanSlopedObjectBeDug(object, position);

	// To check if we're digging from the top, we check the position with a small epsilon above and below
	constexpr float kDigVerticalTolerance = 0.1f;
	const float digYTop    = position.y - kDigVerticalTolerance;
	const float digYBottom = position.y + kDigVerticalTolerance;

	// This accounts for slopes. Nice!
	const Vector2 objectBoundsTopBottom = object->SampleTopBottomAtX(position.x);
	const float objectTop = objectBoundsTopBottom.x;

	return (digYTop <= objectTop && digYBottom >= objectTop);
}

bool PhysicsManager::CanSlopedObjectBeDug(LevelObject* object, const Vector2& position) const
{
	constexpr float kSlopeDigTolerance = 1.0f;
	const float digY = position.y;

	const Vector2 objectBoundsTopBottom = object->SampleTopBottomAtX(position.x);
	const float objectTop    = objectBoundsTopBottom.x;
	const float objectBottom = objectBoundsTopBottom.y;

	return digY <= objectTop + kSlopeDigTolerance && digY >= objectBottom - kSlopeDigTolerance;
}

bool PhysicsManager::IsTestObjectInValidPosition(const LevelObject* testObj) const
{
	if (!testObj) return false;

	Rectangle testBounds = testObj->GetBounds();

	for (LevelObject* solid : _solidObjects)
	{
		if (testObj == solid) continue;
		Rectangle overlap = GetCollisionRec(testBounds, solid->GetBounds());
		if (overlap.width > kTestTolerance && overlap.height > kTestTolerance) return false;
	}

	for (LevelObject* slope : _solidMaybeSlopedObjects)
	{
		if (testObj == slope) continue;
		if (slope->HasActualSlopedHitbox())
		{
			const Rectangle sb = slope->GetBounds();
			const float cx = testBounds.x + testBounds.width  * 0.5f;
			const float cy = testBounds.y + testBounds.height * 0.5f;

			if (cx >= sb.x && cx <= sb.x + sb.width)
			{
				const Vector2 tb = slope->SampleTopBottomAtX(cx);
				const float overlap = std::min(testBounds.y + testBounds.height, tb.y) - std::max(testBounds.y, tb.x);
				if (overlap > kTestTolerance) return false;
			}

			if (testObj->GetHitboxType() == HitboxType::Dynamic && cy >= sb.y && cy <= sb.y + sb.height)
			{
				const Vector2 lr = slope->SampleLeftRightAtY(cy);
				const float overlap = std::min(testBounds.x + testBounds.width, lr.y) - std::max(testBounds.x, lr.x);
				if (overlap > kTestTolerance) return false;
			}
		}
		else
		{
			Rectangle overlap = GetCollisionRec(testBounds, slope->GetBounds());
			if (overlap.width > kTestTolerance && overlap.height > kTestTolerance) return false;
		}
	}

	for (LevelObject* breakable : _solidBreakableObjects)
	{
		if (testObj == breakable) continue;
		if (breakable->GetBreakableIsBroken()) continue;
		Rectangle overlap = GetCollisionRec(testBounds, breakable->GetBounds());
		if (overlap.width > kTestTolerance && overlap.height > kTestTolerance) return false;
	}

	if (testObj->GetHitboxType() == HitboxType::Entity) return true; // Entities can overlap partial semisolids, dynamics cannot
	for (LevelObject* semisolid : _semisolidPartialObjects)
	{
		if (testObj == semisolid) continue;
		Rectangle overlap = GetCollisionRec(testBounds, semisolid->GetBounds());
		if (overlap.width > kTestTolerance && overlap.height > kTestTolerance) return false;
	}

	return true;
}