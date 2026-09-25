#ifndef LEVELOBJECTTILESAND_HH
#define LEVELOBJECTTILESAND_HH

#include "LevelObjects/Tiles/LevelObjectTile.hh"

class LevelObjectTileSand : public LevelObjectTile {
public:
	LevelObjectTileSand(const Vector2& position, LevelManager* levelManager);
	~LevelObjectTileSand();

	void Update(const float deltaTime) override;
	void Render(const float deltaTime) const override;
	void RenderBounds(const float deltaTime, const Color& color) const override;

	HitboxType GetHitboxType() const override { return HitboxType::SolidBreakable; }

	LevelObjectTileType GetTileType() const override;

	void BreakableBreak(const LevelObject* other) override;
	bool GetBreakableIsBroken() const override { return _isBroken; }
	float GetBreakableBreakSpeed() const override { return kInteractionVelocity; }

	void OnDug(LevelObject* digger) override { BreakableBreak(digger); }

protected:

private:
	LevelManager* _levelManager{nullptr};

	bool _isBroken{false};
};

#endif