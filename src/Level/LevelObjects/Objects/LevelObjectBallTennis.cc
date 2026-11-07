#include "LevelObjectBallTennis.hh"
#include "Level/Level.hh"

LevelObjectBallTennis::LevelObjectBallTennis(const Vector2& position, Level* level)
    : _level(level)
{
    _bounds = { position.x + kWidth / 2.0f, position.y + kHeight / 2.0f, kWidth, kHeight };
    
	_kickCooldown = 0.0f;
}

void LevelObjectBallTennis::Update(const float deltaTime) 
{
	if (_inTerrain)
	{
		bool centeredOnTargetTile = _level->IsObjectCenteredOnTerrainTile(this, _terrainTargetTileX, _terrainTargetTileY);

		if (centeredOnTargetTile)
		{
			if (!_level->IsTileTerrain(_terrainTargetTileX, _terrainTargetTileY)) _inTerrain = false;
		}
	}

    if (_isGrounded) SetVelocityX(GetVelocityX() * powf(kGroundFriction, deltaTime));

	if (_kickCooldown > 0.0f) _kickCooldown -= deltaTime;

	if (fabsf(GetVelocityX()) < kInteractionStopVelocityX && fabsf(GetVelocityY()) < kInteractionStopVelocityY && _isGrounded)
	{
		_interactionCauseSlot = kInteractionNoCause;
		_interactionDepth = 0;
	}
}

void LevelObjectBallTennis::Render(const float deltaTime) const 
{
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::BallTennis);

    const float spriteSize = 1.0f;

    Rectangle src = { 0.0f, 0.0f, 16.0f, 16.0f };
	if (fabsf(GetVelocityX()) >= GetInteractionVelocityX() || fabsf(GetVelocityY()) >= GetInteractionVelocityY()) src = {0.0f, 16.0f, 16.0f, 16.0f}; 
    Rectangle dst = {
        _bounds.x + _bounds.width / 2.0f - spriteSize / 2.0f,
        _bounds.y + _bounds.height / 2.0f - spriteSize / 2.0f,
        spriteSize,
        spriteSize
    };

    DrawTexturePro(tex, src, dst, { 0.0f, 0.0f }, 0.0f, WHITE);
}

void LevelObjectBallTennis::CollidedWithX(LevelObject* other, float prevVelocityX)
{
	if (_inTerrain) return;

	SetVelocityX(-prevVelocityX * kBounciness);
}

void LevelObjectBallTennis::CollidedWithY(LevelObject* other, float prevVelocityY)
{
	if (_inTerrain) return;

	SetVelocityY(-prevVelocityY * kBounciness);
}

void LevelObjectBallTennis::OnOverlap(LevelObject* other)
{
	if (_inTerrain) return;

	if (LevelObjectDog* dog = dynamic_cast<LevelObjectDog*>(other))
	{
		KickFromDog(dog);
		return;
	}

	if (LevelObjectBallTennis* otherBall = dynamic_cast<LevelObjectBallTennis*>(other))
	{
		BounceOffBall(otherBall);
		return;
	}
}

void LevelObjectBallTennis::KickFromDog(LevelObjectDog* dog)
{
	if (_inTerrain) return;

	if (_kickCooldown > 0.0f) return;

	_interactionCauseSlot = dog->GetInteractionCauseSlot();
	_interactionDepth = NextInteractionDepth(dog->GetInteractionDepth());

	Rectangle dogBounds = dog->GetBounds();
	Rectangle bounds = GetBounds();

	Vector2 dogCenter  = { dogBounds.x + dogBounds.width * 0.5f, dogBounds.y + dogBounds.height * 0.5f };
	Vector2 ballCenter = { bounds.x + bounds.width * 0.5f,       bounds.y + bounds.height * 0.5f };

	float dogVelocityX = fabsf(dog->GetVelocityX());
	float dogVelocityY = fabsf(dog->GetVelocityY());

	Vector2 direction = Vector2Subtract(ballCenter, dogCenter);
	if (Vector2LengthSqr(direction) < 0.0001f) direction = { 1.0f, 0.0f };
	direction = Vector2Normalize(direction);

	float velocityToAddX = direction.x * kKickSpeed * dogVelocityX;
	float velocityToAddY = direction.y * kKickSpeed * dogVelocityY;

	const float dogSpeed = Vector2Length({ dog->GetVelocityX(), dog->GetVelocityY() });
	const float minBounceScale = Clamp(dogSpeed, 0.0f, 1.0f);

	if (direction.y <= 0.0f && velocityToAddY > -kKickSpeedMinUpwardsVelocity)
		velocityToAddY = -kKickSpeedMinUpwardsVelocity * minBounceScale;

	AddVelocityX(velocityToAddX);
	AddVelocityY(velocityToAddY);

	_kickCooldown = kKickCooldownDuration;
}

void LevelObjectBallTennis::BounceOffBall(LevelObjectBallTennis* otherBall)
{
	Rectangle otherBounds = otherBall->GetBounds();
	Rectangle bounds = GetBounds();

	Vector2 otherCenter = { otherBounds.x + otherBounds.width * 0.5f, otherBounds.y + otherBounds.height * 0.5f };
	Vector2 ballCenter  = { bounds.x + bounds.width * 0.5f,           bounds.y + bounds.height * 0.5f };

	Vector2 toMe = Vector2Subtract(ballCenter, otherCenter);
	float distance = Vector2Length(toMe);

	Vector2 normal;
	if (distance < 0.0001f) { normal = { 1.0f, 0.0f }; distance = 0.0f; }
	else normal = Vector2Scale(toMe, 1.0f / distance);

	Vector2 myVelocity       = { GetVelocityX(), GetVelocityY() };
	Vector2 otherVelocity    = { otherBall->GetVelocityX(), otherBall->GetVelocityY() };
	Vector2 relativeVelocity = Vector2Subtract(myVelocity, otherVelocity);

	const float velocityAlongNormal = Vector2DotProduct(relativeVelocity, normal);

	if (velocityAlongNormal < 0.0f)
	{
		const float impulseMagnitude = -(1.0f + kBounciness) * velocityAlongNormal * 0.5f;
		Vector2 impulse = Vector2Scale(normal, impulseMagnitude);
		AddVelocityX(impulse.x);
		AddVelocityY(impulse.y);
	}

	const float myRadius     = bounds.width * 0.5f;
	const float otherRadius  = otherBounds.width * 0.5f;
	const float overlapDepth = (myRadius + otherRadius) - distance;
	if (overlapDepth > 0.0f)
	{
		AddVelocityX(normal.x * overlapDepth * kSeparationSpeed);
		AddVelocityY(normal.y * overlapDepth * kSeparationSpeed);
	}
}

bool LevelObjectBallTennis::NetGet(NetworkLevelObjectDynEntState& s) const 
{
	s.x = _bounds.x; s.y = _bounds.y; s.vx = _velocity.x; s.vy = _velocity.y;
	s.mode = static_cast<uint8_t>(_inTerrain);
	s.flags = _isGrounded ? 64 : 0;
	s.aux = static_cast<uint8_t>(_surfaceTypeStandingOn);
	s.interactionCause = _interactionCauseSlot;
	s.interactionDepth = _interactionDepth;
	s.t0 = _kickCooldown;
	s.terrainTargetTileX = _terrainTargetTileX;
	s.terrainTargetTileY = _terrainTargetTileY;
	return true;
}

void LevelObjectBallTennis::NetSet(const NetworkLevelObjectDynEntState& s) 
{
	_bounds.x = s.x; _bounds.y = s.y;
	_velocity.x = s.vx; _velocity.y = s.vy;
	_inTerrain = s.mode != 0;
	_isGrounded = (s.flags & 64) != 0;
	if (s.interactionCause == kInteractionNoCause || (s.interactionCause >= 0 && s.interactionCause < kMaxSlots)) _interactionCauseSlot = s.interactionCause;
    else _interactionCauseSlot = kInteractionNoCause;
    _interactionDepth = s.interactionDepth;
	if (s.aux >= static_cast<uint8_t>(SurfaceType::None) && s.aux < static_cast<uint8_t>(SurfaceType::Count)) _surfaceTypeStandingOn = static_cast<SurfaceType>(s.aux);
    else _surfaceTypeStandingOn = SurfaceType::None;
	_kickCooldown = s.t0;
	_terrainTargetTileX = std::min(s.terrainTargetTileX, static_cast<uint8_t>(kMaxLevelWidth - 1));
    _terrainTargetTileY = std::min(s.terrainTargetTileY, static_cast<uint8_t>(kMaxLevelHeight - 1));
}

void LevelObjectBallTennis::OnForceField(const LevelObject* field)
{
	if (field->GetInteractionCauseSlot() == kInteractionNoCause) return;

    _interactionCauseSlot = field->GetInteractionCauseSlot();
    _interactionDepth = NextInteractionDepth(field->GetInteractionDepth());
}

void LevelObjectBallTennis::OnEnteredTerrain(uint8_t tileX, uint8_t tileY, float velX, float velY, bool enteredByDigging)
{
    if (_inTerrain) return;

    _terrainTargetTileX = tileX;
    _terrainTargetTileY = tileY;
    _inTerrain = true;

    SetVelocityX(velX);
    SetVelocityY(velY);
}