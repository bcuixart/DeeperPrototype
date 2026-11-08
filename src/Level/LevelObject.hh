#ifndef LEVELOBJECT_HH
#define LEVELOBJECT_HH

#include <raylib.h>
#include <raymath.h>
#include <iostream>
#include <cstdint>

#include "AssetManager.hh"
#include "PhysicsProperties.hh"
#include "PhysicsManager.hh"
#include "NetworkLevelObjectDynEntState.hh"

class Level;

enum class SurfaceType { None, FlatGround, SlopedLeftGround, SlopedRightGround, Count };

#define SOLID_SIDE_NONE 0
#define SOLID_SIDE_LEFT 1
#define SOLID_SIDE_RIGHT 2
#define SOLID_SIDE_TOP 4
#define SOLID_SIDE_BOTTOM 8

static constexpr uint8_t kInteractionNoCause = 0xFF;

enum class GrabbableType : uint8_t{ None, Bone, Squeaky, Count };

class LevelObject {
public:
	LevelObject();
	virtual ~LevelObject();

	virtual void Update(const float deltaTime);
	virtual void Render(const float deltaTime) const;
	virtual void RenderBounds(const float deltaTime, const Color& color) const;

	virtual bool NetGet(NetworkLevelObjectDynEntState& state) const { return false; }
	virtual void NetSet(const NetworkLevelObjectDynEntState& state) {}

	Rectangle GetBounds() const { return _bounds; }
	Vector2 GetPosition() const { return { _bounds.x, _bounds.y }; }
	void SetBoundsX(const float x) { _bounds.x = x; }
	void SetBoundsY(const float y) { _bounds.y = y; }
	virtual Vector2 SampleLeftRightAtY(const float y) const { return { _bounds.x, _bounds.x + _bounds.width }; }
	virtual Vector2 SampleTopBottomAtX(const float x) const { return { _bounds.y, _bounds.y + _bounds.height }; }

	Vector2 GetVelocity() const { return _velocity; }
	float GetVelocityX() const { return _velocity.x; }
	float GetVelocityY() const { return _velocity.y; }
	void SetVelocityX(const float x) { _velocity.x = x; ClampVelocityX(); }
	void SetVelocityY(const float y) { _velocity.y = y; ClampVelocityY(); }
	void AddVelocityX(const float x) { _velocity.x += x; ClampVelocityX(); }
	void AddVelocityY(const float y) { _velocity.y += y; ClampVelocityY(); }

	void ApplyVelocityX(const float deltaTime) { _bounds.x += _velocity.x * deltaTime; }
	void ApplyVelocityY(const float deltaTime) { _bounds.y += _velocity.y * deltaTime; }

	virtual void CollidedWithX(LevelObject* other, float prevVelocityX) { }
	virtual void CollidedWithY(LevelObject* other, float prevVelocityY) { }

	virtual void OnOverlap(LevelObject* other) { }

	bool GetIsGrounded() const { return _isGrounded; }
	void SetIsGrounded(const bool grounded, SurfaceType surfaceType) { _isGrounded = grounded; _surfaceTypeStandingOn = surfaceType; }

	virtual SurfaceType GetSurfaceType() const { return SurfaceType::FlatGround; }

	virtual HitboxType GetHitboxType() const = 0;

	void SetSolidSideCollisionMask(const uint8_t side, const bool mask);
	bool GetSolidSideCollisionMask(const uint8_t side) const { return (_solidSidecollisionMask & side) != 0; }

	virtual bool HasActualSlopedHitbox() const { return false; }

	virtual void OnDug(LevelObject* digger) { }

	virtual void OnEnteredTerrain(uint8_t tileX, uint8_t tileY, float velX, float velY,bool enteredByDigging) { }
	virtual bool IsInTerrain() const { return false; }
	void GetTerrainMoveTarget(int8_t& tileX, int8_t& tileY) const { tileX = _terrainTargetTileX; tileY = _terrainTargetTileY; }
	void SetTerrainMoveTarget(int8_t tileX, int8_t tileY) { _terrainTargetTileX = tileX; _terrainTargetTileY = tileY; }
	float GetTerrainDesiredMoveSpeed() const { return _terrainDesiredMoveSpeed; }

	virtual uint8_t GetInteractionCauseSlot() const { return kInteractionNoCause; }
	virtual uint8_t GetInteractionDepth() const { return 0; }
	static uint8_t NextInteractionDepth(uint8_t d) { return d < 250 ? d + 1 : d; }
	virtual float GetInteractionVelocityX() const { return PhysicsManager::kTerminalVelocityX; };
	virtual float GetInteractionVelocityY() const { return PhysicsManager::kTerminalVelocityY; };

	virtual void BreakableBreak(const LevelObject* other) { }
	virtual bool GetBreakableIsBroken() const { return false; }
	virtual void NetworkSetBroken(bool broken) { }

	virtual bool GetForceFieldIsActive() const { return false; }
	virtual Vector2 GetForceFieldForce() const { return { 0.0f, 0.0f }; }
	virtual void OnForceField(const LevelObject* field) { }

	virtual void OnTrampoline(const LevelObject* trampoline, float prevVelocityY) { SetVelocityY(std::min(-prevVelocityY, -PhysicsManager::kMinTrampolineBounceVelocity)); }

	virtual GrabbableType GetGrabbableType() const { return GrabbableType::None; }
	void SetGrabbableIndex(uint8_t index) { _grabbableIndex = index; }
	uint8_t GetGrabbableIndex() const { return _grabbableIndex; }

	void AddRenderOffset(const Vector2& d) { _renderOffset.x += d.x; _renderOffset.y += d.y; }
	void ClearRenderOffset() { _renderOffset = { 0.0f, 0.0f }; }
	void DecayRenderOffset(const float frameDt);

protected:
	Rectangle _bounds{};
	Vector2   _velocity{};

	uint8_t _solidSidecollisionMask{ SOLID_SIDE_LEFT | SOLID_SIDE_RIGHT | SOLID_SIDE_TOP | SOLID_SIDE_BOTTOM };

	bool _isGrounded{ false };
	SurfaceType _surfaceTypeStandingOn{ SurfaceType::FlatGround };

	static constexpr float kRenderOffsetDecay = 12.0f;
	Vector2 _renderOffset{ 0.0f, 0.0f };

	int8_t _terrainTargetTileX{ 0 };
	int8_t _terrainTargetTileY{ 0 };
	float _terrainDesiredMoveSpeed{ 0.0f };

	static constexpr int kNotGrabbableIndex = 255;
	uint8_t _grabbableIndex{kNotGrabbableIndex};

private:
	void ClampVelocityX() { if (_velocity.x > PhysicsManager::kTerminalVelocityX) _velocity.x = PhysicsManager::kTerminalVelocityX; if (_velocity.x < -PhysicsManager::kTerminalVelocityX) _velocity.x = -PhysicsManager::kTerminalVelocityX; }
	void ClampVelocityY() { if (_velocity.y > PhysicsManager::kTerminalVelocityY) _velocity.y = PhysicsManager::kTerminalVelocityY; if (_velocity.y < -PhysicsManager::kTerminalVelocityY) _velocity.y = -PhysicsManager::kTerminalVelocityY; }
};

#endif