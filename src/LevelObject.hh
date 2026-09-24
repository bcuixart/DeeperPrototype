#ifndef LEVELOBJECT_HH
#define LEVELOBJECT_HH

#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <cstdint>

#include "AssetManager.hh"
#include "PhysicsProperties.hh"

class LevelManager;

#define SOLID_SIDE_NONE 0
#define SOLID_SIDE_LEFT 1
#define SOLID_SIDE_RIGHT 2
#define SOLID_SIDE_TOP 4
#define SOLID_SIDE_BOTTOM 8

class LevelObject {
public:
	LevelObject();
	virtual ~LevelObject();

	virtual void Update(const float deltaTime);
	virtual void Render(const float deltaTime) const;
	virtual void RenderBounds(const float deltaTime, const Color& color) const;

	Rectangle GetBounds() const { return _bounds; }
	Vector2 GetPosition() const { return { _bounds.x, _bounds.y }; }
	void SetBoundsX(const float x) { _bounds.x = x; }
	void SetBoundsY(const float y) { _bounds.y = y; }
	virtual Vector2 SampleLeftRightAtY(const float y) const { return { _bounds.x, _bounds.x + _bounds.width }; }
	virtual Vector2 SampleTopBottomAtX(const float x) const { return { _bounds.y, _bounds.y + _bounds.height }; }

	Vector2 GetVelocity() const { return _velocity; }
	float GetVelocityMagnitude() const { return Vector2Length(_velocity); }
	float GetVelocityX() const { return _velocity.x; }
	float GetVelocityY() const { return _velocity.y; }
	void SetVelocityX(const float x) { _velocity.x = x; }
	void SetVelocityY(const float y) { _velocity.y = y; }
	void AddVelocityX(const float x) { _velocity.x += x; }
	void AddVelocityY(const float y) { _velocity.y += y; }

	void ApplyVelocityX(const float deltaTime) { _bounds.x += _velocity.x * deltaTime; }
	void ApplyVelocityY(const float deltaTime) { _bounds.y += _velocity.y * deltaTime; }

	virtual void CollidedWithX(LevelObject* other, float prevVelocityX) { }
	virtual void CollidedWithY(LevelObject* other, float prevVelocityY) { }

	virtual void OnOverlap(LevelObject* other) { }

	bool GetIsGrounded() const { return _isGrounded; }
	void SetIsGrounded(const bool grounded) { _isGrounded = grounded; }

	virtual HitboxType GetHitboxType() const = 0;

	void SetSolidSideCollisionMask(const uint8_t side, const bool mask);
	bool GetSolidSideCollisionMask(const uint8_t side) const { return (_solidSidecollisionMask & side) != 0; }

	virtual bool HasActualSlopedHitbox() const { return false; }

	virtual void BreakableBreak(const LevelObject* other) { }
	virtual bool GetBreakableIsBroken() const { return false; }
	virtual float GetBreakableBreakSpeed() const { return 30.0f; }

protected:
	Rectangle _bounds{};
	Vector2   _velocity{};

	uint8_t _solidSidecollisionMask{ SOLID_SIDE_LEFT | SOLID_SIDE_RIGHT | SOLID_SIDE_TOP | SOLID_SIDE_BOTTOM };

	bool _isGrounded{ false };

private:

};

#endif