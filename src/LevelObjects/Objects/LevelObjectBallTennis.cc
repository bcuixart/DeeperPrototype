#include "LevelObjectBallTennis.hh"

LevelObjectBallTennis::LevelObjectBallTennis(const Vector2& position)
{
    _bounds = { position.x + kWidth / 2.0f, position.y + kHeight / 2.0f, kWidth, kHeight };
    
	_kickCooldown = 0.0f;
}

void LevelObjectBallTennis::Update(const float deltaTime) 
{
    if (_isGrounded) _velocity.x *= kGroundFriction;

	if (_kickCooldown > 0.0f) _kickCooldown -= deltaTime;
}

void LevelObjectBallTennis::Render(const float deltaTime) const 
{
    const Texture2D& tex = AssetManager::instance->GetTexture(TextureId::BallTennis);

    const float spriteSize = 1.0f;

    Rectangle src = { 0.0f, 0.0f, 16.0f, 16.0f };
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
	SetVelocityX(-prevVelocityX * kBounciness);
}

void LevelObjectBallTennis::CollidedWithY(LevelObject* other, float prevVelocityY)
{
	SetVelocityY(-prevVelocityY * kBounciness);
}

void LevelObjectBallTennis::OnOverlap(LevelObject* other)
{
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
	if (_kickCooldown > 0.0f) return;

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